# -ffunction-sections -fdata-sections -Wl,–gc-sections 
#GPPPARAMS = -ffunction-sections -fdata-sections -Wl,–gc-sections  -msse -msse2 -msse3 -mfpmath=sse -g -O2 -ffat-lto-objects -flto -ffreestanding -Wno-write-strings -Wno-register -m32 -fno-use-cxa-atexit -nostdlib -fno-builtin -fno-rtti -fno-exceptions -fno-leading-underscore -fno-stack-protector
GPPPARAMS = -O2 -ffreestanding -Wall -Wextra -fno-exceptions -fno-rtti -fno-use-cxa-atexit -g -Wno-unused-variable -Wno-write-strings -Wno-unused-parameter
GCCPARAMS = -O2 -ffreestanding -Wall -Wextra -g
ASPARAMS = --32
LDPARAMS = -static-libgcc -ffreestanding -O2 -nostdlib -lgcc -g

ASMFILE = src/boot/loader.asm

ASM_FILES := $(shell find src -type f -name '*.asm')
S_FILES := $(shell find src -type f -name '*.s')
CPP_FILES := $(shell find src -type f -name '*.cpp')
C_FILES := $(shell find src -type f -name '*.c')

compiledObjects := $(ASM_FILES) $(S_FILES) $(CPP_FILES) $(C_FILES)

objects := $(patsubst %.asm,%.o,$(filter %.asm,$(compiledObjects))) \
           $(patsubst %.s,%.o,$(filter %.s,$(compiledObjects))) \
           $(patsubst %.cpp,%.o,$(filter %.cpp,$(compiledObjects))) \
           $(patsubst %.c,%.o,$(filter %.c,$(compiledObjects)))

%.o: %.cpp
	i686-elf-g++ $(GPPPARAMS) -o $@ -c $<

%.o: %.c
	i686-elf-gcc $(GCCPARAMS) -o $@ -c $<

%.o: %.asm # For nasm compiler
	nasm -f elf32 -o $@ $<

%.o: %.s # For legacy GNU compiler
	as $(ASPARAMS) -o $@ $<
	
kernel.bin: linker.ld $(objects)
	i686-elf-g++ -T $< -o ../dist/$@ $(objects) $(LDPARAMS)
	rm $(objects)

build: kernel.bin