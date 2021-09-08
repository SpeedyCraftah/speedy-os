#pragma once

namespace Software {
    class ProcessRuntime {
        public:
            void on_start();
            void on_exit();

            void yield() {
                asm volatile("jmp yield_process");
            }
    };
}