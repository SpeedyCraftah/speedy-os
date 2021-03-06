#include "pic.h"

#include "../abstractions/io_port.h"

// Common PIC definitions.
#define PIC1		0x20
#define PIC2		0xA0
#define PIC1_COMMAND	PIC1
#define PIC1_DATA	(PIC1+1)
#define PIC2_COMMAND	PIC2
#define PIC2_DATA	(PIC2+1)

// PIC configuration.
#define ICW1_ICW4	0x01
#define ICW1_SINGLE	0x02
#define ICW1_INTERVAL4	0x04
#define ICW1_LEVEL	0x08
#define ICW1_INIT	0x10
#define ICW4_8086	0x01
#define ICW4_AUTO	0x02
#define ICW4_BUF_SLAVE	0x08
#define ICW4_BUF_MASTER	0x0C
#define ICW4_SFNM	0x10

void chips::pic::remap(uint8_t master_offset, uint8_t slave_offset) {
    // Save mask states.
    uint8_t a1 = io_port::bit_8::in(PIC1_DATA);
	uint8_t a2 = io_port::bit_8::in(PIC2_DATA);
 
    // Add lots of delays to let possible slow PICs catch up.

    // Starts init sequence for both chips.
	io_port::bit_8::out(PIC1_COMMAND, ICW1_INIT | ICW1_ICW4);
    io_port::bit_8::delay_void();
	io_port::bit_8::out(PIC2_COMMAND, ICW1_INIT | ICW1_ICW4);
    io_port::bit_8::delay_void();

	io_port::bit_8::out(PIC1_DATA, master_offset);
    io_port::bit_8::delay_void();
	io_port::bit_8::out(PIC2_DATA, slave_offset);
    io_port::bit_8::delay_void();
	io_port::bit_8::out(PIC1_DATA, 4);
    io_port::bit_8::delay_void();
	io_port::bit_8::out(PIC2_DATA, 2);
    io_port::bit_8::delay_void();
 
	io_port::bit_8::out(PIC1_DATA, ICW4_8086);
    io_port::bit_8::delay_void();
	io_port::bit_8::out(PIC2_DATA, ICW4_8086);
    io_port::bit_8::delay_void();
 
    // Restore masks.
	io_port::bit_8::out(PIC1_DATA, a1);
	io_port::bit_8::out(PIC2_DATA, a2);
}

void chips::pic::mask_line(uint8_t line) {
    uint16_t port;
    uint8_t value;
 
    // Worker or master?
    if(line < 8) {
        port = PIC1_DATA;
    } else {
        port = PIC2_DATA;
        line -= 8;
    }

    value = io_port::bit_8::in(port) | (1 << line);
    io_port::bit_8::out(port, value);
}

void chips::pic::unmask_line(uint8_t line) {
    uint16_t port;
    uint8_t value;
 
    // Worker or master?
    if (line < 8) {
        port = PIC1_DATA;
    } else {
        port = PIC2_DATA;
        line -= 8;
    }

    value = io_port::bit_8::in(port) & ~(1 << line);
    io_port::bit_8::out(port, value);
}

void chips::pic::send_eoi_master() {
    asm volatile("mov $0x20, %al");
    asm volatile("out %al, $0x20");
}

void chips::pic::send_eoi_slave() {
    io_port::bit_8::out(0x20, 0x20);
    io_port::bit_8::out(0xA0, 0x20);
}