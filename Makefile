.PHONY: kernel.bin
kernel.bin:
	$(MAKE) -C kernel/ kernel.bin

.PHONY: software
software:
	$(MAKE) -C software/ all

.PHONY: speedyos.iso
speedyos.iso: kernel.bin software
	cp ./dist/$< iso/boot/
	grub-mkrescue --output=./dist/$@ iso
	rm iso/boot/kernel.bin

.PHONY: run
run: speedyos.iso

clean:
	find . -name "*.o" -type f -delete
	find . -name "*.bin" -type f -delete
	find . -name "*.iso" -type f -delete