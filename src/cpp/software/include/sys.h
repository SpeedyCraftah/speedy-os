#pragma once

#include <stdint.h>

namespace speedyos {
    enum ThreadExecutionPolicy {
        NORMAL, // Thread will run normally. Events will awake the thread.
        EVENT_ONLY, // Thread will only run upon events. Events will awake the thread.
        BUSY // Thread will ignore events when running. Events will not awake the thread.
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

    enum MouseButtons {
        LEFT_PRESS = 0,
        LEFT_RELEASE = 1,

        RIGHT_PRESS = 2,
        RIGHT_RELEASE = 3,

        MIDDLE_PRESS = 4,
        MIDDLE_RELEASE = 5,
    };

    enum VGAColour : uint32_t {
        BLACK = 0x000000,
        BLUE = 0x0000FF,
        GREEN = 0x00FF00,
        CYAN = 0x00FFFF,
        RED = 0xFF0000,
        MAGENTA = 0xFF00FF,
        BROWN = 0xA52A2A,
        LIGHT_GREY = 0xD3D3D3,
        DARK_GREY = 0x808080,
        LIGHT_BLUE = 0x00b3ff,
        LIGHT_GREEN = 0x90EE90,
        LIGHT_CYAN = 0xE0FFFF,
        LIGHT_RED = 0xFFA07A,
        LIGHT_MAGENTA = 0xFFE4E1,
        LIGHT_BROWN = 0xFFDEAD,
        WHITE = 0xFFFFFF
    };

    // Queries the kernel for the process ID.
    __attribute__((naked)) __attribute__((fastcall)) uint32_t fetch_process_id();

    // Queries the kernerl for the thread ID.
    __attribute__((naked)) __attribute__((fastcall)) uint32_t fetch_thread_id();

    // Queries the kernel for the elapsed time from startup.
    __attribute__((naked)) __attribute__((fastcall)) uint32_t fetch_elapsed_time();

    // Notifies the kernel to stop the process.
    __attribute__((naked)) __attribute__((fastcall)) void end_process(uint32_t code = 0);

    // Notifies the kernel to suspend the process for a specified amount of time.
    __attribute__((naked)) __attribute__((fastcall)) void suspend_thread(uint32_t ms);

    // Notifies the kernel an event has been processed.
    __attribute__((naked)) __attribute__((fastcall)) void end_event();

    // Requests the kernel for a location of specified size (not exactly) in the heap.
    __attribute__((naked)) __attribute__((fastcall)) void* malloc_heap(uint32_t bytes);

    // Requests the kernel to free the specified memory location and returns t/f depending on success.
    __attribute__((naked)) __attribute__((fastcall)) bool free_heap(void* address);

    // Registers the process to another process with specific events and returns t/f depending on success.
    __attribute__((naked)) __attribute__((fastcall)) bool register_event_for_thread(uint32_t target_thread_id, uint32_t enabled_events, void(*handler)(uint32_t, uint32_t));

    // If process an event emitter, will emit an ID and data to registered programs.
    __attribute__((naked)) __attribute__((fastcall)) void emit_event(uint32_t event_id, uint32_t event_data);

    // Requests the kernel to change the threads status.
    __attribute__((naked)) __attribute__((fastcall)) void update_execution_policy(ThreadExecutionPolicy policy, uint32_t thread_id = 0);

    // Requests the kernel to return the process ID of a program. 0 if does not exist.
    __attribute__((naked)) __attribute__((fastcall)) uint32_t fetch_process_id_by_string(char* process_name);

    // Returns a hardware & time entropied random 32-bit number.
    __attribute__((naked)) __attribute__((fastcall)) uint32_t hardware_random();

    // Requests the kernel to create a new process thread.
    __attribute__((naked)) __attribute__((fastcall)) uint32_t create_thread(void (*start)());

    // Requests the kernel to kill the specific thread.
    __attribute__((naked)) __attribute__((fastcall)) void kill_thread(uint32_t thread_id = 0);

    // Requests the kernel to park the specified thread.
    __attribute__((naked)) __attribute__((fastcall)) bool park_thread(uint32_t thread_id = 0);

    // Requests the kernel to awake (unpark) the specified thread.
    __attribute__((naked)) __attribute__((fastcall)) bool awake_thread(uint32_t thread_id);

    // Requests the kernel to upgrade the graphics to pixel mode.
    // This will place SpeedyShell in a read-only mode.
    __attribute__((naked)) __attribute__((fastcall)) bool upgrade_graphics();

    namespace speedyshell {
        // Requests input. If at start of program, command will be returned. Otherwise run-time input will be returned if requested.
        __attribute__((naked)) __attribute__((fastcall)) char* fetch_input();

        // Sends a message in the SpeedyShell terminal.
        __attribute__((naked)) __attribute__((fastcall)) void printf(char* text, VGAColour colour = VGAColour::WHITE);
    
        // Notifies the kernel & SpeedyShell to suspend the process until an input is submitted.
        __attribute__((naked)) __attribute__((fastcall)) void request_input();
    }
}