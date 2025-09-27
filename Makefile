EXNAME = CHIP8_EMU
CC = gcc
CSTD = -std=gnu23
CFLAGS = -Wall \
         -Werror \
         -Wextra \
		 -MMD \
		 -MP
PROD_FLAGS    = -DVERSION_STR="\"0.0.1\"" \
							  -DDBG_ENABLE=1
TEST_FLAGS    = -DDBG_ENABLE=1 \
						    -DVERSION_STR="\"0.0.1_t\""
TEST_EN       = 1
CARGS         := $(CSTD) $(CFLAGS) $(if $(filter 1, $(TEST_EN)), $(TEST_FLAGS),$(PROD_FLAGS))
INCLUDE_DIRS  = include/
DEPS          = sdl3
DEPS_CMD      = `pkg-config --cflags --libs $(DEPS)`
CCMD         := $(CC) $(CARGS) -I$(INCLUDE_DIRS) $(DEPS_CMD) 
BUILD_DIR     = build
SRC_DIR       = src
SRC           = emu.c\
                chip8.c\
                opt.c\
                graphics/display.c\
                utility/utils.c
			
TEST_SRC      = emu.c\
                chip8.c\
                opt.c\
                graphics/display.c\
                utility/utils.c

#SRC := $(patstub %.c,src/%.c,$(SRC))
# for each file in SRC or TEST_SRC, create the corrisponding object file in build
OBJS := $(patsubst %.c,$(BUILD_DIR)/%.o,\
									$(if $(filter 1,$(TEST_EN)),$(TEST_SRC),$(SRC)))
DEPS := $(OBJS:.o=.d)



all : $(OBJS)
	$(CCMD) -o $(EXNAME) $(OBJS)

# ogni file oggetto nella build dipende dal suo corrispettivo file c nel source
# ricorda che le regole scritte qua non tengono conto se i file nelle cartelle ci sono o no, make infatti per ogni prerequisito di all cercherà nel makefile la regola da seguire , essendoci qua un %, qualunque file terminante con .o e contenuto in BUILD_DIR seguirà questa regola
$(BUILD_DIR)/%.o : $(SRC_DIR)/%.c | $(BUILD_DIR)
	# creiamo la directory di quel file oggetto nella build
	mkdir -p $(dir $@)
	# compiliamo il file, ci genera anche le dipendenze dagli header
	$(CCMD) -o $@ $< -c $(CARGS)

$(BUILD_DIR):
	mkdir -p $(BUILD_DIR)

.PHONY : test
test : $(OBJS)
	$(CCMD) -o $(EXNAME)_test $(OBJS)

.PHONY : clean
clean :
	rm -rf $(BUILD_DIR)/ $(EXNAME)


# facciamo gestire le dipendenze dagli header
-include $(DEPS)
