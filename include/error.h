#ifndef __ERROR_H__
#define __ERROR_H__

#if defined __linux__
  #include <unistd.h>
#else
	__error("This tool is only usable on LINUX systems!!");
#endif


#define ERR_COLOR       "\033[31m"
#define ERR_COLOR_INFO  "\033[36m"
#define ERR_COLOR_EV    "\033[46m"
#define ERR_COLOR_RESET "\033[m"


#if DBG_ENABLE > 0
#define emu_die(emu, FORMAT, arg...) do{\
  free_emu(emu);\
	int __tty__ = isatty(fileno(stderr));\
	const char* __ce__ = __tty__ ? ERR_COLOR : "";\
	const char* __ci__ = __tty__ ? ERR_COLOR_INFO : "";\
	fprintf(stderr, "%sdie%s at %s(%u) " FORMAT "\n", __ce__, __ci__, __FILE__, __LINE__, ## arg);\
	if( __tty__ ) fputs(ERR_COLOR_RESET, stderr);\
	fflush(stderr);\
	fsync(3);\
	exit(1);\
}while(0)

#define die(FORMAT, arg...) do{\
	int __tty__ = isatty(fileno(stderr));\
	const char* __ce__ = __tty__ ? ERR_COLOR : "";\
	const char* __ci__ = __tty__ ? ERR_COLOR_INFO : "";\
	fprintf(stderr, "%sdie%s at %s(%u) " FORMAT "\n", __ce__, __ci__, __FILE__, __LINE__, ## arg);\
	if( __tty__ ) fputs(ERR_COLOR_RESET, stderr);\
	fflush(stderr);\
	fsync(3);\
	exit(1);\
}while(0)

#else
#define emu_die(emu, FORMAT, arg...) do{\
  free_emu(emu);\
	int __tty__ = isatty(fileno(stderr));\
	const char* __ce__ = __tty__ ? ERR_COLOR : "";\
	const char* __ci__ = __tty__ ? ERR_COLOR_INFO : "";\
	fprintf(stderr, "%sdie%s " FORMAT "\n", __ce__, __ci__, ## arg);\
	if( __tty__ ) fputs(ERR_COLOR_RESET, stderr);\
	exit(1);\
}while(0)


#define die(FORMAT, arg...) do{\
	int __tty__ = isatty(fileno(stderr));\
	const char* __ce__ = __tty__ ? ERR_COLOR : "";\
	const char* __ci__ = __tty__ ? ERR_COLOR_INFO : "";\
	fprintf(stderr, "%sdie%s " FORMAT "\n", __ce__, __ci__, ## arg);\
	if( __tty__ ) fputs(ERR_COLOR_RESET, stderr);\
	exit(1);\
}while(0)

#endif


#endif
