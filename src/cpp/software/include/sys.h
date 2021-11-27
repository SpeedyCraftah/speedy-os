#pragma once

#include <stdint.h>

namespace speedyos {
    enum SuspensionType {
        FULL = 0, // Program will be completely suspended (including events).
        EVENTS_ONLY = 1 // Base-program suspension. Events will still receive CPU time.
    };

    enum TaskStatus : uint8_t {
        RUNNING = 0, // Process will run both during events and on its own accord.
        RUNNING_WAITING_FOR_DATA = 2, // Process will only run and handle events.
        RUNNING_BUSY = 3  // Process will only run on its own accord and will ignore incoming events.
    };

    enum ModifierKeys {
        ENTER_PRESSED = 0x1C,
        ENTER_RELEASED = 0x9C,

        ESCAPE_PRESSED = 0x01,
        ESCAPE_RELEASED = 0x81,

        BACKSPACE_PRESSED = 0x0E,
        BACKSPACE_RELEASED = 0x8E,

        CAPSLOCK_PRESSED = 0x3A,
        CAPSLOCK_RELEASED = 0xBA,

        LEFTSHIFT_PRESSED = 0x2A,
        LEFTSHIFT_RELEASED = 0xAA,

        LEFTCTRL_PRESSED = 0x1D,
        LEFTCTRL_RELEASED = 0x9D
    };

    // Queries the kernel for the process ID.
    __attribute__((naked)) __attribute__((fastcall)) uint32_t fetch_process_id();

    // Queries the kernel for the elapsed time from startup.
    __attribute__((naked)) __attribute__((fastcall)) uint32_t fetch_elapsed_time();

    // Notifies the kernel to stop the process.
    __attribute__((naked)) __attribute__((fastcall)) void end_process(uint32_t code = 0);

    // Notifies the kernel to suspend the process for a specified amount of time.
    __attribute__((naked)) __attribute__((fastcall)) void suspend_process(uint32_t ms, SuspensionType type);

    // Notifies the kernel an event has been processed.
    __attribute__((naked)) __attribute__((fastcall)) void end_event();

    // Requests the kernel for a location of specified size (not exactly) in the heap.
    __attribute__((naked)) __attribute__((fastcall)) void* malloc_heap(uint32_t bytes);

    // Requests the kernel to free the specified memory location and returns t/f depending on success.
    __attribute__((naked)) __attribute__((fastcall)) bool free_heap(void* address);

    // Registers the process to another process with specific events and returns t/f depending on success.
    __attribute__((naked)) __attribute__((fastcall)) bool register_event_for_process(uint32_t target_process_id, uint32_t enabled_events, void(__attribute__((fastcall)) *handler)(uint32_t, uint32_t));

    // If process an event emitter, will emit an ID and data to registered programs.
    __attribute__((naked)) __attribute__((fastcall)) void emit_event(uint32_t event_id, uint32_t event_data);

    // Requests the kernel to change the processes status.
    __attribute__((naked)) __attribute__((fastcall)) void update_status(TaskStatus new_status);

    // Requests the kernel to return the process ID of a program. 0 if does not exist.
    __attribute__((naked)) __attribute__((fastcall)) uint32_t fetch_process_id_by_string(char* process_name);
}