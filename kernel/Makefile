# -ffunction-sections -fdata-sections -Wl,–gc-sections 
#GPPPARAMS = -ffunction-sections -fdata-sections -Wl,–gc-sections  -msse -msse2 -msse3 -mfpmath=sse -g -O2 -ffat-lto-objects -flto -ffreestanding -Wno-write-strings -Wno-register -m32 -fno-use-cxa-atexit -nostdlib -fno-builtin -fno-rtti -fno-exceptions -fno-leading-underscore -fno-stack-protector
GPPPARAMS = -D_SHARED_KERNEL -O0 -ffreestanding -Wall -Wextra -fno-exceptions -fno-rtti -fno-use-cxa-atexit -ggdb -Wno-unused-variable -Wno-write-strings -Wno-unused-parameter
GCCPARAMS = -D_SHARED_KERNEL -O0 -ffreestanding -Wall -Wextra -ggdb
ASPARAMS = --32
LDPARAMS = -D_SHARED_KERNEL -static-libgcc -ffreestanding -O2 -nostdlib -lgcc -ggdb

SRC_DIR = src
DIST_DIR = dist
SHARED_DIR = ../shared

ASM_FILES := $(shell find $(SRC_DIR) -type f -name '*.asm')
S_FILES := $(shell find $(SRC_DIR) -type f -name '*.s')
CPP_FILES := $(shell find $(SRC_DIR) -type f -name '*.cpp')
C_FILES := $(shell find $(SRC_DIR) -type f -name '*.c')
SHARED_FILES := $(shell find $(SHARED_DIR) -type f -name '*.cpp')

compiledObjects := $(ASM_FILES) $(S_FILES) $(CPP_FILES) $(C_FILES) $(SHARED_FILES)

objects := $(patsubst $(SRC_DIR)/%.asm,$(DIST_DIR)/%.o,$(filter $(SRC_DIR)/%.asm,$(compiledObjects))) \
           $(patsubst $(SRC_DIR)/%.s,$(DIST_DIR)/%.o,$(filter $(SRC_DIR)/%.s,$(compiledObjects))) \
           $(patsubst $(SRC_DIR)/%.cpp,$(DIST_DIR)/%.o,$(filter $(SRC_DIR)/%.cpp,$(compiledObjects))) \
           $(patsubst $(SRC_DIR)/%.c,$(DIST_DIR)/%.o,$(filter $(SRC_DIR)/%.c,$(compiledObjects))) \
		   $(patsubst $(SHARED_DIR)/%.cpp,$(DIST_DIR)/%.o,$(filter $(SHARED_DIR)/%.cpp,$(compiledObjects))) \

$(DIST_DIR)/%.o: $(SRC_DIR)/%.cpp
	@mkdir -p $(dir $@)
	i686-elf-g++ $(GPPPARAMS) -o $@ -c $<

$(DIST_DIR)/%.o: $(SRC_DIR)/%.c
	@mkdir -p $(dir $@)
	i686-elf-gcc $(GCCPARAMS) -o $@ -c $<

$(DIST_DIR)/%.o: $(SRC_DIR)/%.asm
	@mkdir -p $(dir $@)
	nasm -I ./src/asm/scheduling/ -f elf32 -o $@ $<

$(DIST_DIR)/%.o: $(SRC_DIR)/%.s
	@mkdir -p $(dir $@)
	as $(ASPARAMS) -o $@ $<

# Shared.
$(DIST_DIR)/%.o: $(SHARED_DIR)/%.cpp
	@mkdir -p $(dir $@)
	i686-elf-g++ $(GPPPARAMS) -o $@ -c $<

kernel.bin: linker.ld $(objects)
	i686-elf-g++ -T $< -o $(DIST_DIR)/$@ $(objects) $(LDPARAMS)

build: kernel.bin

.PHONY: clean
clean:
	rm -rf $(DIST_DIR)