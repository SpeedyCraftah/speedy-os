#include "vga_registers.h"

namespace vga_registers {
   void set_value(uint16_t port, uint8_t index, uint8_t value) {
     if (port == 0x3C0) {
       // Select index state.
       io_port::bit_8::in(0x3DA);
       
       // Send index.
       io_port::bit_8::out(0x3C0, index);
       
       // Send data.
       io_port::bit_8::out(0x3C0, value);
     } else if (port == 0x3C2) {
       // Send data.
       io_port::bit_8::out(0x3C2, value);
     } else if (port == 0x3C4 || port == 0x3CE || port == 0x3D4) {
       // Send index.
       io_port::bit_8::out(port, index);
       
       // Send data.
       io_port::bit_8::out(port + 1, value);
     } else if (port == 0x3C6) {
       // Send data.
       io_port::bit_8::out(0x3C6, value);
     }
   }
  
   uint8_t read_value(uint16_t port, uint8_t index) {
     if (port == 0x3C0) {
       // Select index state.
       io_port::bit_8::in(0x3DA);
       
       // Send index.
       io_port::bit_8::out(0x3C0, index);
       
       // Read data.
       uint8_t data = io_port::bit_8::in(0x3C1);
       
       // Ensure byte order.
       io_port::bit_8::in(0x3DA);
     } else if (port == 0x3C2) {
       // Read data.
       return io_port::bit_8::in(0x3C2);
     } else if (port == 0x3C4 || port == 0x3CE || port == 0x3D4) {
       // Send index.
       io_port::bit_8::out(port, index);
       
       // Read data.
       return io_port::bit_8::in(port + 1);
     } else if (port == 0x3C6) {
       // Read data.
       return io_port::bit_8::in(0x3C6);
     }
   }
}
