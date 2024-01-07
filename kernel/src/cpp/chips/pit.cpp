#include "pit.h"

#include "../abstractions/io_port.h"
#include "../io/video.h"

// Define the divisor to divide the frequency by.
#define DIVISOR 1193180

void chips::pit::set_channel_0_frequency(uint16_t frequency_hz) {
    // Calculate tick number.
    uint16_t ct_number = DIVISOR / frequency_hz;

    // Send command initialisation code for channel 0.
    io_port::bit_8::out(0x43, 0x34);    

    // Split the number by two 8 bits since this is an 8 bit line.      
    io_port::bit_8::out(0x40, ct_number & 0xFF);  
    io_port::bit_8::out(0x40, ct_number >> 8);
}