#include "mouse.h"

#include "stdint.h"
#include "../../abstractions/io_port.h"
#include "../../scheduling/scheduler.h"
#include "../../scheduling/events.h"
#include "../../misc/math.h"

#define MOUSE_PORT   0x60
#define MOUSE_STATUS 0x64
#define MOUSE_ABIT   0x02
#define MOUSE_BBIT   0x01
#define MOUSE_WRITE  0xD4
#define MOUSE_F_BIT  0x20
#define MOUSE_V_BIT  0x08

// Mouse driver events.
// ID 1 - On mouse button press.
// ID 2 - On mouse button release.
// ID 4 - On mouse XY movement.

namespace drivers {
    static Process* process;

    bool mouse::left_button_pressed = false;
    bool mouse::right_button_pressed = false;
    bool mouse::middle_button_pressed = false;

    void mouse::load() {
        // Create virtual process for events to take place.
        ProcessFlags flags;
        flags.system_process = true;
        flags.virtual_process = true;

        process = scheduler::create_process(
            "Mouse Driver",
            0,
            flags
        );

        // Enable AUX input.
        wait(1);
        io_port::bit_8::out(MOUSE_STATUS, 0xA8);
        wait(1);

        // Set COMPAQ status.
        io_port::bit_8::out(MOUSE_STATUS, 0x20);
        wait(0);

        // Returned status byte.
        // Set second bit (enable IRQ12), unset bit 5 (disable mouse clock).
        uint8_t status = io_port::bit_8::in(0x60) | (2 & ~0x20);
        wait(1);

        // Send command byte.
        io_port::bit_8::out(MOUSE_STATUS, 0x60);
        wait(1);

        io_port::bit_8::out(MOUSE_PORT, status);
        write(0xF6);
        read();

        write(0xF4);
        read();
    }

    void mouse::handle_interrupt() {
        uint8_t status = io_port::bit_8::in(MOUSE_STATUS);
        uint8_t packet = 0;

        int8_t x = 0;

        while (status & MOUSE_BBIT) {
            int8_t data = io_port::bit_8::in(MOUSE_PORT);
            status = io_port::bit_8::in(MOUSE_STATUS);

            if (packet == 0) {
                // Left button status.
                if ((data & 1) != 0) {
                    if (!left_button_pressed) {
                        scheduler::events::emit_event(process, 1, MOUSE_BUTTONS::LEFT_PRESS);
                    }

                    left_button_pressed = true;
                } else {
                    if (left_button_pressed) {
                        scheduler::events::emit_event(process, 2, MOUSE_BUTTONS::LEFT_RELEASE);
                    }

                    left_button_pressed = false;
                }

                // Right button status.
                if ((data & 2) != 0) {
                    if (!right_button_pressed) {
                        scheduler::events::emit_event(process, 1, MOUSE_BUTTONS::RIGHT_PRESS);
                    }

                    right_button_pressed = true;
                } else {
                    if (right_button_pressed) {
                        scheduler::events::emit_event(process, 2, MOUSE_BUTTONS::RIGHT_RELEASE);
                    }

                    right_button_pressed = false;
                }

                // Middle button status.
                if ((data & 4) != 0) {
                    if (!middle_button_pressed) {
                        scheduler::events::emit_event(process, 1, MOUSE_BUTTONS::MIDDLE_PRESS);
                    }

                    middle_button_pressed = true;
                } else {
                    if (middle_button_pressed) {
                        scheduler::events::emit_event(process, 2, MOUSE_BUTTONS::MIDDLE_RELEASE);
                    }

                    middle_button_pressed = false;
                }

            // X axis movement.
            } else if (packet == 1) {
                x = data;

            // Y axis movement.
            // Both movements collected.
            } else if (packet == 2) {
                if (data != 0 || x != 0) {
                    // Number bit formation (left being low bits).
                    // X sign, Y sign, X movement, Y movement.
                    uint32_t movement_data = 0;

                    // X byte.
                    movement_data |= math::abs(x);

                    // Y byte.
                    movement_data <<= 8;
                    movement_data |= math::abs(data);

                    // X and Y sign bits.
                    if (x > 0) movement_data |= 0x20000;
                    if (data > 0) movement_data |= 0x10000;

                    scheduler::events::emit_event(process, 4, movement_data);
                }
            }

            packet++;
        }
    }

    void mouse::write(uint8_t data) {
        wait(1);
        io_port::bit_8::out(MOUSE_STATUS, MOUSE_WRITE);
        wait(1);
        io_port::bit_8::out(MOUSE_PORT, data);
    }

    uint8_t mouse::read() {
        wait(0);
        char t = io_port::bit_8::in(MOUSE_PORT);
        return t;
    }

    void mouse::wait(bool type) {
        uint32_t timeout = 100000;
        if (!type) {
            while (--timeout) {
                if ((io_port::bit_8::in(MOUSE_STATUS) & MOUSE_BBIT) == 1) {
                    return;
                }
            }
            return;
        } else {
            while (--timeout) {
                if (!((io_port::bit_8::in(MOUSE_STATUS) & MOUSE_ABIT))) {
                    return;
                }
            }
            return;
        }
    }
}