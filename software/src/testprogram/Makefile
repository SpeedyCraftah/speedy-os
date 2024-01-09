GPP_PARAMS = -O0 -ffreestanding -Wall -Wextra -fno-exceptions -fno-rtti -fno-use-cxa-atexit -ggdb -Wno-unused-variable -Wno-write-strings -Wno-unused-parameter -fstack-protector-all
LINKER_PARAMS = -fstack-protector-all -static-libgcc -ffreestanding -O2 -nostdlib -lgcc -ggdb -Wl,--gc-sections

DIST_FOLDER := ../../dist/$(shell basename "$(CURDIR)")
PROGRAM_BIN := $(DIST_FOLDER)/$(shell basename "$(CURDIR)").bin
GRUB_MOD_PATH := ../../../iso/boot/software/
LINKER_FILE := ../../linker.ld
CPP_FILES := $(shell find . -type f -name '*.cpp')
INCLUDE_BIN := $(shell find ../../include -type f -name '*.o')
compiledObjects := $(CPP_FILES)
objects := $(patsubst %.cpp,$(DIST_FOLDER)/%.o,$(filter %.cpp,$(compiledObjects)))

$(DIST_FOLDER)/%.o: %.cpp
	mkdir -p $(dir $@)
	i686-elf-g++ $(GPP_PARAMS) -o $@ -c $< 

link: $(LINKER_FILE) $(objects)
	i686-elf-g++ -T $< -o $(PROGRAM_BIN) $(objects) $(INCLUDE_BIN) $(LINKER_PARAMS)

build: link

install: $(PROGRAM_BIN)
	cp $(PROGRAM_BIN) $(GRUB_MOD_PATH)

.NOTPARALLEL: clean
clean:
	rm -rf $(DIST_FOLDER)