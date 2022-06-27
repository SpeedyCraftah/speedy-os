# -ffunction-sections -fdata-sections -Wl,–gc-sections 
#GPPPARAMS = -ffunction-sections -fdata-sections -Wl,–gc-sections  -msse -msse2 -msse3 -mfpmath=sse -g -O2 -ffat-lto-objects -flto -ffreestanding -Wno-write-strings -Wno-register -m32 -fno-use-cxa-atexit -nostdlib -fno-builtin -fno-rtti -fno-exceptions -fno-leading-underscore -fno-stack-protector
GPPPARAMS = -O2 -ffreestanding -Wall -Wextra -fno-exceptions -fno-rtti -fno-use-cxa-atexit -g -Wno-unused-variable -Wno-write-strings -Wno-unused-parameter
GCCPARAMS = -O2 -ffreestanding -Wall -Wextra -g
ASPARAMS = --32
LDPARAMS = -static-libgcc -ffreestanding -O2 -nostdlib -lgcc -g

ASMFILE = src/boot/loader.asm

compiledObjects = $(wildcard src/*.asm) $(wildcard src/*/*.asm) $(wildcard src/*/*/*.asm) $(wildcard src/*.s) $(wildcard src/*/*.s) $(wildcard src/*/*/*.s) $(wildcard src/*.cpp) $(wildcard src/*/*.cpp) $(wildcard src/*/*/*.cpp) $(wildcard src/*/*/*/*.cpp) $(wildcard src/*/*/*/*/*.cpp) $(wildcard src/*/*/*/*/*/*.cpp) $(wildcard src/*/*/*/*/*/*/*.cpp) $(wildcard src/*.c) $(wildcard src/*/*.c) $(wildcard src/*/*/*.c) $(wildcard src/*/*/*/*.c) $(wildcard src/*/*/*/*/*.c) $(wildcard src/*/*/*/*/*/*.c)

# replace objects1 extensions from cpp to o.
replacedCpp = $(subst .cpp,.o,$(compiledObjects))
replacedC = $(subst .c,.o,$(replacedCpp))
replacedAsm = $(subst .asm,.o,$(replacedC))
objects = $(subst .s,.o,$(replacedAsm))

%.o: %.cpp
	i686-elf-g++ $(GPPPARAMS) -o $@ -c $<

%.o: %.c
	i686-elf-gcc $(GCCPARAMS) -o $@ -c $<

%.o: %.asm # For nasm compiler
	nasm -f elf32 -o $@ $<

%.o: %.s # For legacy GNU compiler
	as $(ASPARAMS) -o $@ $<
	
#ld $(LDPARAMS) -T $< -o ./dist/$@ $(objects)	
kernel.bin: src/linker.ld $(objects)
	i686-elf-g++ -T $< -o ./dist/$@ $(objects) $(LDPARAMS)
	rm $(objects)

install: kernel.bin
	sudo cp $< /boot/kernel.bin
	
speedyos.iso: kernel.bin
	cp ./dist/$< iso/boot/
	grub-mkrescue --output=./dist/$@ iso
	rm iso/boot/kernel.bin
	

run: speedyos.iso
	
clean:
	find . -name "*.o" -type f -delete