# -ffunction-sections -fdata-sections -Wl,–gc-sections 
GPPPARAMS = -O2 -ffreestanding -Wno-write-strings -Wno-register -m32 -fno-use-cxa-atexit -nostdlib -fno-builtin -fno-rtti -fno-exceptions -fno-leading-underscore -fno-stack-protector
ASPARAMS = --32
LDPARAMS = -melf_i386

ASMFILE = src/boot/loader.asm

compiledObjects = $(wildcard src/*.asm) $(wildcard src/*/*.asm) $(wildcard src/*/*/*.asm) $(wildcard src/*.s) $(wildcard src/*/*.s) $(wildcard src/*/*/*.s) $(wildcard src/*.cpp) $(wildcard src/*/*.cpp) $(wildcard src/*/*/*.cpp) $(wildcard src/*/*/*/*.cpp)

# replace objects1 extensions from cpp to o.
replacedCpp = $(subst .cpp,.o,$(compiledObjects))
replacedAsm = $(subst .asm,.o,$(replacedCpp))
objects = $(subst .s,.o,$(replacedAsm))

%.o: %.cpp
	export PATH="$$HOME/opt/cross/bin:$$PATH"
	i686-elf-g++ $(GPPPARAMS) -o $@ -c $<

%.o: %.asm # For nasm compiler
	nasm -f elf32 -o $@ $<

%.o: %.s # For legacy GNU compiler
	as $(ASPARAMS) -o $@ $<
	
kernel.bin: src/linker.ld $(objects)
	ld $(LDPARAMS) -T $< -o ./dist/$@ $(objects)
	rm $(objects)

install: kernel.bin
	sudo cp $< /boot/kernel.bin
	
speedyos.iso: kernel.bin
	mkdir iso
	mkdir iso/boot
	mkdir iso/boot/grub
	cp ./dist/$< iso/boot/
	echo 'set timeout-0' >> iso/boot/grub/grub.cfg
	echo 'set default-0' >> iso/boot/grub/grub.cfg
	echo 'menuentry "Speedy OS v0.1"{' >> iso/boot/grub/grub.cfg
	echo '  multiboot /boot/kernel.bin' >> iso/boot/grub/grub.cfg
	echo ' boot' >> iso/boot/grub/grub.cfg
	echo '}' >> iso/boot/grub/grub.cfg
	grub-mkrescue --output=./dist/$@ iso
	rm -rf iso 

run: speedyos.iso
	