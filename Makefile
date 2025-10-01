EXNAME = CHIP8_EMU
CC = gcc
CSTD = -std=gnu23
CFLAGS = -Wall \
         -Werror \
         -Wextra \
         -Wno-int-to-pointer-cast\
		     -MMD \
		     -MP
PROD_FLAGS    = -DVERSION_STR="\"1.0.0\"" \
							  -DDBG_ENABLE=1
TEST_FLAGS    = -DDBG_ENABLE=1 \
						    -DVERSION_STR="\"1.0.0_t\""
TEST_EN       = 0
CARGS         := $(CSTD) $(CFLAGS) $(if $(filter 1, $(TEST_EN)), $(TEST_FLAGS),$(PROD_FLAGS))
INCLUDE_DIRS  = include/
DEPS          = sdl3
DEPS_CMD      = `pkg-config --cflags --libs $(DEPS)`
CCMD         := $(CC) $(CARGS) -I$(INCLUDE_DIRS) $(DEPS_CMD) 
BUILD_DIR     = build
SRC_DIR       = src
SRC           = emu.c\
                chip8.c\
                instr.c\
                opt.c\
                graphics/display.c\
                utility/utils.c
			
TEST_SRC      = emu.c\
                chip8.c\
                instr.c\
                opt.c\
                graphics/display.c\
                utility/utils.c

OBJS := $(patsubst %.c,$(BUILD_DIR)/%.o,\
									$(if $(filter 1,$(TEST_EN)),$(TEST_SRC),$(SRC)))
DEPS := $(OBJS:.o=.d)



all : $(OBJS)
	$(CCMD) -o $(EXNAME) $(OBJS)

$(BUILD_DIR)/%.o : $(SRC_DIR)/%.c | $(BUILD_DIR)
	mkdir -p $(dir $@)
	$(CCMD) -o $@ $< -c $(CARGS)

$(BUILD_DIR):
	mkdir -p $(BUILD_DIR)

.PHONY : test
test : $(OBJS)
	$(CCMD) -o $(EXNAME)_test $(OBJS)

.PHONY : clean
clean :
	rm -rf $(BUILD_DIR)/ $(EXNAME)


-include $(DEPS)
