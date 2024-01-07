; Provides methods which can be called from C++ for video (VGA).

global disable_vga_cursor
disable_vga_cursor:
	pushf
	push eax
	push edx
 
	mov dx, 0x3D4
	mov al, 0xA
	out dx, al
 
	inc dx
	mov al, 0x20
	out dx, al
 
	pop edx
	pop eax
	popf
	ret

global fix_blinking_text
fix_blinking_text:
	mov dx, 0x03DA
	in al, dx

	mov dx, 0x03C0
	mov al, 0x30
	out dx, al

	inc dx
	in al, dx

	and al, 0xF7

	dec dx
	out dx, al
	ret