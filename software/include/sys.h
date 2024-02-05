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
        LEFTCTRL_RELEASED = 0x9D,

        CURSOR_UP_PRESSED = 0xFF + 0x48,
        CURSOR_UP_RELEASED = 0xFF + 0xC8,

        CURSOR_DOWN_PRESSED = 0xFF + 0x50,
        CURSOR_DOWN_RELEASED = 0xFF + 0xD0,

        CURSOR_LEFT_PRESSED = 0xFF + 0x4B,
        CURSOR_LEFT_RELEASED = 0xFF + 0xCB,

        CURSOR_RIGHT_PRESSED = 0xFF + 0x4D,
        CURSOR_RIGHT_RELEASED = 0xFF + 0xCD
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

    enum PageAllocationFlag : uint32_t {
        ALLOCATE_ON_ACCESS = 1, // Virtual address space is reserved but physical page will only be allocated on access (page fault).
        ALLOCATE_IMMEDIATELY = 2, // Physical page will be allocated immediately on request alongside the virtual page.
    };

    enum PageFreeFlag : uint32_t {
        ONLY_FREE_PHYSICAL // Virtual address will not be freed but if a physical page is allocated it will be freed.
    };

    // Queries the kernel for the process ID.
    __attribute__((naked)) __attribute__((fastcall)) uint32_t fetch_process_id();

    // Queries the kernerl for the thread ID.
    __attribute__((naked)) __attribute__((fastcall)) uint32_t fetch_thread_id();

    // Queries the kernel for the elapsed time from startup.
    __attribute__((naked)) __attribute__((fastcall)) uint32_t fetch_elapsed_time();

    // Notifies the kernel to stop the process.
    __attribute__((naked)) __attribute__((fastcall)) __attribute__((noreturn)) void end_process(uint32_t code = 0);

    // Notifies the kernel to suspend the process for a specified amount of time.
    __attribute__((naked)) __attribute__((fastcall)) void suspend_thread(uint32_t ms);

    // Notifies the kernel an event has been processed.
    __attribute__((naked)) __attribute__((fastcall)) void end_event();

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

    // Requests the kernel to createa a new process thread with a capture value.
    // Ensure the pointer value is not released or copied before the thread ends otherwise undefined behaviour will occur.
    __attribute__((naked)) __attribute__((fastcall)) uint32_t create_thread(void (*start)(void*), void* capture = 0);

    // Requests the kernel to kill the specific thread.
    __attribute__((naked)) __attribute__((fastcall)) void kill_thread(uint32_t thread_id = 0);

    // Requests the kernel to park the specified thread.
    __attribute__((naked)) __attribute__((fastcall)) bool park_thread(uint32_t thread_id = 0);

    // Requests the kernel to awake (unpark) the specified thread.
    __attribute__((naked)) __attribute__((fastcall)) bool awake_thread(uint32_t thread_id);

    // Requests the kernel to upgrade the graphics to pixel mode.
    // This will place SpeedyShell in a read-only mode.
    // This will map the framebuffer to a virtual page and return the address of the page.
    __attribute__((naked)) __attribute__((fastcall)) uint32_t* upgrade_graphics();

    // Notifies the kernel that the thread has voluntarily given up execution.
    __attribute__((naked)) __attribute__((fastcall)) void preempt_thread();

    // Requests the kernel to allocate a number of consecutive virtual pages with optional flags.
    // If the call cannot be completed a null pointer will be returned.
    __attribute__((naked)) __attribute__((fastcall)) void* alloc_virtual_pages(void* requested_address, uint32_t count, uint32_t flags = 0);

    // Requests the kernel to free a specified page with optional flags.
    // Returns a boolean indicating the success of the operation.
    // Attempting to free an address which is not page aligned will raise a fault.
    __attribute__((naked)) __attribute__((fastcall)) bool free_virtual_page(void* address, uint32_t flags = 0);

    // Writes data to the end of a data sink.
    // Returns a boolean indicating the success of the operation.
    // The sink will not be modified if a status of FALSE is returned. 
    __attribute__((naked)) __attribute__((fastcall)) bool write_steady_datasink(uint32_t sink_id, uint8_t* data, uint32_t data_size);

    // Reads data FIFO from the datasink to the desired buffer of X size.
    // This will disregard fragments and read fragments as if they were whole.
    // Returns an int indicating -1 for error, 0 for no data available/read, otherwise indicating the length read which may be <= data_size.
    __attribute__((naked)) __attribute__((fastcall)) int read_steady_datasink(uint32_t sink_id, uint8_t* dest, uint32_t read_size);

    // Fetches the size of the latest fragment from the data sink.
    // Returns an int indicating -1 for error, 0 for no data available, otherwise indicating the size of the latest fragment.
    __attribute__((naked)) __attribute__((fastcall)) int fetch_fragment_size_steady_datasink(uint32_t sink_id);

    // Reads the latest fragment from the data sink and writes it to the destination buffer.
    // Returns an int indicating -1 for error/no data available, otherwise indicating the size of the next fragment (0 if none).
    // This function assumes that you have fetched the size of the latest fragment using fetch_fragment_size_steady_datasink and the destination buffer is of appropriate size.
    __attribute__((naked)) __attribute__((fastcall)) int read_fragment_steady_datasink(uint32_t sink_id, uint8_t* dest);

    namespace speedyshell {
        // Requests input. If at start of program, command will be returned. Otherwise run-time input will be returned if requested.
        __attribute__((naked)) __attribute__((fastcall)) char* fetch_input();

        // Sends a message in the SpeedyShell terminal.
        // Proxy function.
        __attribute__((fastcall)) void printf(char* text, VGAColour colour = VGAColour::WHITE);
    
        // Notifies the kernel & SpeedyShell to suspend the process until an input is submitted.
        __attribute__((naked)) __attribute__((fastcall)) void request_input();
    }

    // High level functions.
    void __attribute__((noreturn)) panic(char* error);
}