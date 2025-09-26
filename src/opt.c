#include <string.h>
#include <errno.h>
#include <stdlib.h>
#include <stdio.h>
#include <opt.h>
#include <sys/stat.h>
#include <utility/utils.h>
#include <error.h>

//#if DBG_ENABLE > 0
#define dbg_info printf
//#endif

const void *dbgptr;

typedef struct optctx{
	option_s* opt;
	char**    argv;
	unsigned  argc;
	unsigned  current;
	unsigned  paarg;
	unsigned  paoff;
	unsigned  count;
	int       unmanaged;
}optctx_s;

typedef struct kv{
	char* name;
	char* value;
}kv_s;


// counts the amount of options in the array, last option is marked with OPT_END flag
__private unsigned opt_count(option_s* opt){
	unsigned copt = 0;
	while( !(opt[copt++].flags & OPT_END) );
	return copt;
}

// separa opzione=valore solo in caso di opzioni lunghe(--opzione=valore)
__private void kv_parse(kv_s* kv, const char* arg){
	kv->value = strchrnul(arg, '='); // se non lo trova ritorna un puntatore al carattere nul della stringa, e non a NULL; così possiamo sicuramente dereferenziarlo (dereferenziare NULL infatti è ub)
	kv->name  = str_dup(arg, kv->value - arg); // prendiamo il nome del parametro
	if( *kv->value == '=' ) ++kv->value; // prendiamo il valore
}

// gestisce gli errori originati dalle opzioni specificate dall'utente
__private void opt_die(optctx_s* ctx, const char* desc){
	unsigned starg = 0; // tiene traccia della posizione di inizio dell'argomento corrente 
	unsigned stcur = 0;
  // stampiamo tutti gli argomenti parsati correttamente fino ad arriare a quello che ha dato l'errore
	for( unsigned i = 0; i < ctx->argc; ++i ){
		unsigned n = fprintf(stderr, "%s ", ctx->argv[i]);
		if( i < ctx->paarg ) starg += n;
		if( i < ctx->current ) stcur += n;
	}
	fputc('\n', stderr);
  // stampa spazi nella riga sotto per arrivare all'inizio dell'argomento errore
	for( unsigned i = 0; i < starg; ++i ) fputc(' ', stderr);

	for( unsigned i = 0; i < ctx->paoff; ++i ) fputc(' ', stderr);
  // stampa una freccia proprio sotto l'opzione errore
	fputc('^', stderr);
  // se stavamo già parsando il valore di quell'opzione , allora current > paarg
	if( ctx->paarg < ctx->current ){
    // vuol dire che l'errore lo ha dato il valore, e non l'opzione stessa, quindi mettiamo freccina anche sotto il valore
		starg += ctx->paoff;
		for( unsigned i = starg; i < stcur - 1; ++i)  fputc('~', stderr);
		fputc('^', stderr);
	}
	fputc('\n', stderr);
	die("error argument: %s", desc); // NOLINT
}

// imposta un'opzione nell array di opzioni disponibili come presente(specificata dall'utente)
__private void opt_set(optctx_s* ctx, int id){
	if( id == -1 ){ // se non è un opzione disponibile
		ctx->current = ctx->paarg; // l'opzione corrente è quella che stavamo parsando
		opt_die(ctx, "unknow option"); // chiamiamo l'handler degli errori
	}
	option_s* opt = &ctx->opt[id]; 
	++opt->set; // settiamola
  // se l'opzione era già stata specificata e ci può essere solo una volta(non ha la flag REPEAT) andiamo in errore
	if( opt->set > 1 && !(opt->flags & OPT_REPEAT) ) opt_die(ctx, "unacepted repeated option");
}


// Ritorna l'indice di una determinata opzione nell'array di opzioni confrontando il nome lungo
__private int find_long(optctx_s* ctx, const char* lo){
	for( unsigned i = 0; i < ctx->count; ++i ){
		if( !strcmp(ctx->opt[i].l_opt, lo) ) return i;
	}
	return ctx->unmanaged;
}

// Ritorna l'indice di una determinata opzione nell'array di opzioni confrontando il nome corto
__private int find_short(optctx_s* ctx, const char sh){
	for( unsigned i = 0; i < ctx->count; ++i ){
		if( ctx->opt[i].s_opt == sh ) return i;
	}
	return ctx->unmanaged;
}

// VEDERE FIND_LONG E POI PROSEGUIRE CON METTERE LE FUNZIONI CHE MANCANO
// TESTARE E CAPIRE paoff CHE CAZZO E'
// controlla se un determinato elemento successivo ad un opzione nella lista argv sia un altra opzione(visto che non è un valore, vedi add_to_option )
__private void next_is_nopt(optctx_s* ctx, unsigned next){
  // se la lista è finita
	if( next >= ctx->argc ){
		dbg_info("next > argc");
		goto ONERR;
	}
  // prendiamo il prossimo argomento in argv
	const char* str = ctx->argv[next];
	if( str[0] == '-' ){
		if( str[1] == '-' ){
      // se questo argomento è anch'esso un'opzione lunga e non è tra quelle non gestite OPT_EXTRA, allora errore, in quanto abbiamo 2 opzioni attaccate(e la prima invece voleva un valore)
			if( find_long(ctx, str) != ctx->unmanaged ){
				dbg_info("long option %s exists", str);
				goto ONERR;
			}
		}
		else{ // se invece c'è un opzione corta valida (non OPT_EXTRA o non esistente), errore
			const char* p = &str[2];
			while( *p ){
				if( find_short(ctx, *p) != ctx->unmanaged ){
					dbg_info("short option %c exists", *p);
					goto ONERR;
				}
				++p;
			}
		}
	}
	else{ // errore anche se c'è un opzione senza trattini lunga
		if( find_long(ctx, str) != ctx->unmanaged ){
			dbg_info("long option --- %s exists	", str);
			goto ONERR;
		}
	}

	return;
ONERR:
	opt_die(ctx, "option required value");
}


__private char* opt_array(const char** arr){
  // se l'array non è già stato elaborato tutto
	if( !**arr ) return NULL;
  // estraiamo il prossimo elemento fino alla virgola
	const char* s = *arr;
	const char* e = strchrnul(s, ',');
  // incrementiamo il puntatore all'array per mandarlo sull'argomento successivo, finchè esiste, altrimenti nul
	*arr = *e ? e+1 : e;
  // ritorna una copia dell'elemento isolato
	return str_dup(s, e-s);
}

// adds an element to array linked list options
__private optval_u* opt_value_new(option_s* opt) {
  // expand options' array
	opt->value = realloc(opt->value, sizeof(optval_u) * (opt->set + 1));
  if(!opt->value) {
   strerror(errno);
   die("Memory error while expanding options array");
  }
  return (opt->value)+(opt->set);
}


// Parsa il numero senza segno
__private unsigned long opt_parse_num(optctx_s* ctx, const char* value){
	errno = 0;
	if( *value == '-' ) opt_die(ctx, "option aspected unsigned value");
	char* end;
	unsigned long num = strtoul(value, &end, 0);
	if( !end || *end || errno ) opt_die(ctx, "option aspected unsigned value");
	return num;
}

// parsa il numero sena segno
__private long opt_parse_inum(optctx_s* ctx, const char* value){
	errno = 0;
	char* end;
	long num = strtol(value, &end, 0);
	if( !end || *end || errno ) opt_die(ctx, "option aspected signed value");
	return num;
}


// parsa un percorso controllando se esiste
__private char* opt_parse_path(optctx_s* ctx, char* value, unsigned flags){
	if( flags & OPT_EXISTS ){
		struct stat sb; // stat prende delle info su un determinato file e le mette in una struttura stat
		if (stat(value, &sb) < 0) opt_die(ctx, "option aspected a valid path");
		if( flags & OPT_DIR ){ // se è una cartella
      // S_ISDIR è una macro GNU che ritorna non-zero se il campo st_mode re,ativo ad un file è una directory valida
			if( !S_ISDIR(sb.st_mode) ) opt_die(ctx, "option aspected a valid dir");
		}
		else{
      // stessa cosa, ma ritorna non-zero se è un regular file
			if( !S_ISREG(sb.st_mode) ) opt_die(ctx, "option aspected a valid file");
		}
	}
	return value;
}


// estrae il valore dell'opzione nel relativo tipo
__private void opt_value(optctx_s* ctx, unsigned id, const char* value){
	if( ctx->opt[id].flags & OPT_ARRAY ){ // se l'opzione è un array di valori
		char* v;
		--ctx->opt[id].set; // diciamo che l'opzione non è settata (0 parametri settati)
		while( (v=opt_array(&value)) ){ // finchè ci sono valori in questo array
      // in base al valore si estrae e gestisce
			switch( ctx->opt[id].flags & OPT_TYPE ){
        case OPT_STR : opt_value_new(ctx->opt+id)->str = str_dup(v, 0); break;
				case OPT_NUM : opt_value_new(ctx->opt+id)->i  = opt_parse_num(ctx, v); break;
				case OPT_INUM: opt_value_new(ctx->opt+id)->ui  = opt_parse_inum(ctx, v); break;
				case OPT_PATH: opt_value_new(ctx->opt+id)->str = opt_parse_path(ctx, (char*)v, ctx->opt[id].flags); break;
				default: die("internal error, unaspected option type, report this error"); break;
			}
			++ctx->opt[id].set;
			free(v);
		}
	}
	else{
		switch( ctx->opt[id].flags & OPT_TYPE ){
      case OPT_STR : ctx->opt[id].value->str = value; break;
			case OPT_NUM : ctx->opt[id].value->i  = opt_parse_num(ctx, value); break;
			case OPT_INUM: ctx->opt[id].value->ui  = opt_parse_inum(ctx, value); break;
			case OPT_PATH: ctx->opt[id].value->str = opt_parse_path(ctx, (char*)value, ctx->opt[id].flags); break;
			default: die("internal error, unaspected option type, report this error"); break;
		}
	}
}







__private void add_to_option(optctx_s* ctx, int id, kv_s* kv){
  // setta l'opzione come utilizzata tra l'array di tutte le disponibili
	opt_set(ctx, id);
  // se l'opzione in analisi deve ricevere un valore
	if( (ctx->opt[id].flags & OPT_TYPE) != OPT_NOARG ){
		char* v = NULL;
    // se si sta analizzando una short option, e quindi alla chiamata kv viene passato NULL
		if( !kv || !*kv->value ){
      // controlliamo che dopo la short option attuale non ci sia un'altra opzione, in caso generiamo errore
			next_is_nopt(ctx, ++ctx->current);
      // se è tutto apposto, allora possiamo dire che l'argomento trovato dopo la short option è il suo valore
			v = ctx->argv[ctx->current];
		}
		else{ // se invece viene già passata una struttura kv, vuol dire che stavamo parsando una long option(con l'=) e che quindi il suo valore l'abbiamo già estratto ed è in kv->value
			v = kv->value;
		}
    // impostiamo questo valore nell'array di opzioni
		opt_value(ctx, id, v);
	}
  // se invece l'opzione non vuole un valore ma la struttura kv è piena. vuol dire che gli è stato dato ed è quindi stato parsato, errore
	else if( kv && *kv->value ){
		opt_die(ctx, "optiont unaspected value");
	}
}

// POI QUA
// parsa le opzioni lunghe
__private void long_option(optctx_s* ctx){
	ctx->paarg = ctx->current; // stiamo parsando l'argomento current ovvero il prossimo da parsare
	kv_s kv;
	kv_parse(&kv, ctx->argv[ctx->current]); // parsiamo l'opzione in nome e valore
	add_to_option(ctx, find_long(ctx, kv.name), &kv);
	free(kv.name);
  ++ctx->current;
}

// parsa le opzioni corte
__private void short_option(optctx_s* ctx){
	const char* opt = ctx->argv[ctx->current];
	++opt;
	ctx->paarg = ctx->current;
	ctx->paoff = 1; //perchè escludiamo il trattino, quindi aggiungiamo 1 all'offset
  //continuiamo ad incrementare l'offset finchè non troviamo un'opzione valida
	while( *opt ){
		add_to_option(ctx, find_short(ctx, *opt), NULL);
		++opt;
		++ctx->paoff;
	}
	++ctx->current;
	ctx->paoff = 0;
}

// fills the available options array with actually user specified options
option_s* argv_parse(option_s* opt, int argc, char** argv, char **prog_name) {
  const unsigned copt = opt_count(opt);
  optctx_s ctx = {
		.argc      = argc,
		.argv      = argv,
		.current   = 0, // prossimo argomento da essere parsato
		.paarg     = 0, // current argument being parsed
		.paoff     = 0, // offset , in termini di caratteri, dall'inizio dell'opzione corrente(del nome, non del valore)
		.opt       = opt,
		.count     = copt,
		.unmanaged = -1
	};

	for( unsigned i = 0; i < copt; ++i ){
		if( opt[i].flags & OPT_EXTRA ) ctx.unmanaged = i;
		opt[i].value = (optval_u*)malloc(sizeof(optval_u));
		opt[i].set = 0;
	}


	ctx.current = 1;
	while( ctx.current < (unsigned)argc ){
		if( argv[ctx.current][0] == '-' ){
			if( argv[ctx.current][1] == '-' ){
        // 2 dashes, long option
				long_option(&ctx);
			}
			else{
				short_option(&ctx);
			}
		}
    // if no dashes at all it's a file
		else if(!*prog_name) 
      *prog_name = str_dup(argv[ctx.current++], 0);
	}

	return opt;
}

// libera la memoria allocata per un'opzione(anche il suo array se c'è)
option_s* argv_dtor(option_s* opt){
	if( !opt ) return NULL;
	const unsigned count = opt_count(opt);
	for( unsigned i = 0; i < count; ++i ){
    if( opt[i].value ) {
      if( opt[i].set && (opt[i].flags & OPT_ARRAY) && (opt[i].flags & OPT_STR) ) 
        for(size_t j = 0; j < opt[i].set; j++)
          free((char *)(opt[i].value[j]).str); 
      free(opt[i].value);
    }
	}
	return opt;
}

void argv_cleanup(void* ppopt){
	argv_dtor(*(void**)ppopt);
}



void argv_usage(option_s* opt, const char* argv0){
	const unsigned count = opt_count(opt);
	printf("%s [options] ", argv0);
	for( unsigned i = 0; i < count; ++i ){
		if( opt[i].flags & OPT_EXTRA ){
			puts("[extra]");
			printf("extra:\n%s", opt[i].desc);
			break;
		}
	}
	puts("\nv" VERSION_STR);// NOLINT 
	puts("options:");
	for( unsigned i = 0; i < count; ++i ){
		if( !opt[i].s_opt && !*opt[i].l_opt ) continue;
		if( opt[i].s_opt ) printf("-%c ", opt[i].s_opt);
		if( *opt[i].l_opt ) printf("%s ", opt[i].l_opt);
		if( opt[i].flags & OPT_REPEAT ) printf("<can repeat this option>");
		if( opt[i].flags & OPT_ARRAY  ) printf("<can use as array>");
		if( opt[i].flags & OPT_EXTRA  ) printf("<accept not option value>");
		if( opt[i].flags & OPT_EXISTS ) printf("<path need exists>");
		if( opt[i].flags & OPT_DIR    ) printf("<is dir>");
		switch( opt[i].flags & OPT_TYPE ){
			case OPT_NOARG: puts("<not required argument>"); break;
			case OPT_STR  : puts("<required string>"); break;
			case OPT_NUM  : puts("<required unsigned integer>"); break;
			case OPT_INUM : puts("<required integer>"); break;
			case OPT_PATH : puts("<required path>"); break;
			default: die("internal error, report this message");
		}
		putchar('\t');
		puts(opt[i].desc);
	}
	exit(0);
}


void print_args(option_s *opt) {
  size_t count = opt_count(opt);
  for(size_t i = 0; i < count; i++) {
    if(!opt[i].s_opt && !*opt[i].l_opt) continue;
    printf("Arg: %s, -%c, value: ", opt[i].l_opt, opt[i].s_opt);
    if(!opt[i].set) {
      puts("Not set");
      continue;
    }
    for(size_t k = 0; k < opt[i].set; k++) {
      optval_u *curr_val = opt[i].value+k;
      switch(opt[i].flags & OPT_TYPE) {
        case OPT_NOARG: printf("specified with no arg"); break;
        case OPT_STR: printf("%s ", curr_val->str); break;
        case OPT_NUM: printf("%lu ", curr_val->ui); break;
        case OPT_INUM: printf("%ld ", curr_val->i); break;
        case OPT_PATH: printf("PATH: %s ", curr_val->str); break;
        default: die("internal error, report this message");
      }
    }
    puts("");
  }
  fflush(stdout);
}
