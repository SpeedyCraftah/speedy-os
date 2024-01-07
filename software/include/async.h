#pragma once

#include "alloc.h"
#include "misc/conversions.h"
#include "sys.h"

// Async implementation for programs in SpeedyOS
// where long-lived tasks can be off-loaded to another thread in a single line.
// Warning! Chain responsibly otherwise the object may unexpectedly have the destructer called.

template <class ReturnT>
class async {
    public:
        template <typename FunctionT>
        async(FunctionT func) {
            function = reinterpret_cast<void*>(func);
            parent_thread_id = speedyos::fetch_thread_id();
        }

        async& exec() {
            // Start the thread and call the function.
            thread_id = speedyos::create_thread([](void* capture_ptr) {
                auto self = (async<ReturnT>*)capture_ptr;
                auto func = reinterpret_cast<ReturnT (*)()>(self->function);
                self->return_value = func();
                self->complete = true;

                // If thread is waiting for operation to complete, wake it up.
                if (self->waiting) speedyos::awake_thread(self->parent_thread_id);

                return speedyos::kill_thread();
            }, this);

            return *this;
        }

        template <typename T1>
        async& exec(T1 t1) {
            struct capture_t {
                async<ReturnT>* self;
                T1 t1;
            };
            
            capture_t* c = new (malloc(sizeof(capture_t))) capture_t;
            c->self = this;
            c->t1 = t1;

            param_ptr = (void*)c;

            // Start the thread and call the function.
            thread_id = speedyos::create_thread([](void* capture_ptr) {
                auto capture = (capture_t*)capture_ptr;
                auto func = reinterpret_cast<ReturnT (*)(T1)>(capture->self->function);
                capture->self->return_value = func(capture->t1);
                capture->self->complete = true;

                // If thread is waiting for operation to complete, wake it up.
                if (capture->self->waiting) speedyos::awake_thread(capture->self->parent_thread_id);

                // Free parameter pointer.
                free(capture_ptr);

                return speedyos::kill_thread();
            }, c);

            return *this;
        }

        template <typename T1, typename T2>
        async& exec(T1 t1, T2 t2) {
            struct capture_t {
                async<ReturnT>* self;
                T1 t1;
                T2 t2;
            };
            
            capture_t* c = new (malloc(sizeof(capture_t))) capture_t;
            c->self = this;
            c->t1 = t1;
            c->t2 = t2;

            param_ptr = (void*)c;

            // Start the thread and call the function.
            thread_id = speedyos::create_thread([](void* capture_ptr) {
                auto capture = (capture_t*)capture_ptr;
                auto func = reinterpret_cast<ReturnT (*)(T1, T2)>(capture->self->function);
                capture->self->return_value = func(capture->t1, capture->t2);
                capture->self->complete = true;

                // If thread is waiting for operation to complete, wake it up.
                if (capture->self->waiting) speedyos::awake_thread(capture->self->parent_thread_id);

                // Free parameter pointer.
                free(capture_ptr);

                return speedyos::kill_thread();
            }, c);

            return *this;
        }

        template <typename T1, typename T2, typename T3>
        async& exec(T1 t1, T2 t2, T3 t3) {
            struct capture_t {
                async<ReturnT>* self;
                T1 t1;
                T2 t2;
                T3 t3;
            };
            
            capture_t* c = new (malloc(sizeof(capture_t))) capture_t;
            c->self = this;
            c->t1 = t1;
            c->t2 = t2;
            c->t3 = t3;

            param_ptr = (void*)c;

            // Start the thread and call the function.
            thread_id = speedyos::create_thread([](void* capture_ptr) {
                auto capture = (capture_t*)capture_ptr;
                auto func = reinterpret_cast<ReturnT (*)(T1, T2, T3)>(capture->self->function);
                capture->self->return_value = func(capture->t1, capture->t2, capture->t3);
                capture->self->complete = true;

                // If thread is waiting for operation to complete, wake it up.
                if (capture->self->waiting) speedyos::awake_thread(capture->self->parent_thread_id);

                // Free parameter pointer.
                free(capture_ptr);

                return speedyos::kill_thread();
            }, c);

            return *this;
        }

        template <typename T1, typename T2, typename T3, typename T4>
        async& exec(T1 t1, T2 t2, T3 t3, T4 t4) {
            struct capture_t {
                async<ReturnT>* self;
                T1 t1;
                T2 t2;
                T3 t3;
                T4 t4;
            };
            
            capture_t* c = new (malloc(sizeof(capture_t))) capture_t;
            c->self = this;
            c->t1 = t1;
            c->t2 = t2;
            c->t3 = t3;
            c->t4 = t4;

            param_ptr = (void*)c;

            // Start the thread and call the function.
            thread_id = speedyos::create_thread([](void* capture_ptr) {
                auto capture = (capture_t*)capture_ptr;
                auto func = reinterpret_cast<ReturnT (*)(T1, T2, T3)>(capture->self->function);
                capture->self->return_value = func(capture->t1, capture->t2, capture->t3, capture->t4);
                capture->self->complete = true;

                // If thread is waiting for operation to complete, wake it up.
                if (capture->self->waiting) speedyos::awake_thread(capture->self->parent_thread_id);

                // Free parameter pointer.
                free(capture_ptr);

                return speedyos::kill_thread();
            }, c);

            return *this;
        }

        // Parks and blocks the thread until the value is resolved.
        async& await() {
            waiting = true;

            // Park the thread until completion.
            while (!complete) {
                speedyos::park_thread();
            }

            return *this;
        }

        // Returns the value.
        // Panics if the thread has not completed.
        ReturnT value() {
            if (!complete) {
                speedyos::speedyshell::printf("[FATAL] A value has been requested in a promise while it has not been completed.");
                speedyos::end_process(1);
                __builtin_unreachable();
            }

            return return_value;
        }

        // Returns a reference to the value.
        // Panics if the thread has not completed.
        ReturnT& value_ref() {
            if (!complete) {
                speedyos::speedyshell::printf("[FATAL] A value has been requested in a promise while it has not been completed.");
                speedyos::end_process(1);
                __builtin_unreachable();
            }

            return return_value;
        }

        // Returns whether the operation is complete.
        bool completed() {
            return complete;
        }

        ~async() {
            // If thread exists, kill.
            if (thread_id != 0 && !complete) speedyos::kill_thread(thread_id);
            if (param_ptr != nullptr && !complete) free(param_ptr);
        }
    private:
        ReturnT return_value;
        bool complete = false;
        bool waiting = false;
        uint32_t thread_id = 0;
        uint32_t parent_thread_id = 0;
        void* function;
        void* param_ptr = nullptr;
};

// Async implementation for programs in SpeedyOS
// where long-lived tasks can be off-loaded to another thread in a single line.
// This is a no-return version.

class async_void {
    public:
        template <typename FunctionT>
        async_void(FunctionT func) {
            function = reinterpret_cast<void*>(func);
            parent_thread_id = speedyos::fetch_thread_id();
        }

        async_void& exec() {
            // Start the thread and call the function.
            thread_id = speedyos::create_thread([](void* capture_ptr) {
                auto self = (async_void*)capture_ptr;
                auto func = reinterpret_cast<void (*)()>(self->function);
                func();
                self->complete = true;

                // If thread is waiting for operation to complete, wake it up.
                if (self->waiting) speedyos::awake_thread(self->parent_thread_id);

                return speedyos::kill_thread();
            }, this);

            return *this;
        }

        template <typename T1>
        async_void& exec(T1 t1) {
            struct capture_t {
                async_void* self;
                T1 t1;
            };
            
            capture_t* c = new (malloc(sizeof(capture_t))) capture_t;
            c->self = this;
            c->t1 = t1;

            param_ptr = (void*)c;

            // Start the thread and call the function.
            thread_id = speedyos::create_thread([](void* capture_ptr) {
                auto capture = (capture_t*)capture_ptr;
                auto func = reinterpret_cast<void (*)(T1)>(capture->self->function);
                func(capture->t1);
                capture->self->complete = true;

                // If thread is waiting for operation to complete, wake it up.
                if (capture->self->waiting) speedyos::awake_thread(capture->self->parent_thread_id);

                // Free parameter pointer.
                free(capture_ptr);

                return speedyos::kill_thread();
            }, c);

            return *this;
        }

        template <typename T1, typename T2>
        async_void& exec(T1 t1, T2 t2) {
            struct capture_t {
                async_void* self;
                T1 t1;
                T2 t2;
            };
            
            capture_t* c = new (malloc(sizeof(capture_t))) capture_t;
            c->self = this;
            c->t1 = t1;
            c->t2 = t2;

            param_ptr = (void*)c;

            // Start the thread and call the function.
            thread_id = speedyos::create_thread([](void* capture_ptr) {
                auto capture = (capture_t*)capture_ptr;
                auto func = reinterpret_cast<void (*)(T1, T2)>(capture->self->function);
                capture->self->return_value = func(capture->t1, capture->t2);
                capture->self->complete = true;

                // If thread is waiting for operation to complete, wake it up.
                if (capture->self->waiting) speedyos::awake_thread(capture->self->parent_thread_id);

                // Free parameter pointer.
                free(capture_ptr);

                return speedyos::kill_thread();
            }, c);

            return *this;
        }

        template <typename T1, typename T2, typename T3>
        async_void& exec(T1 t1, T2 t2, T3 t3) {
            struct capture_t {
                async_void* self;
                T1 t1;
                T2 t2;
                T3 t3;
            };
            
            capture_t* c = new (malloc(sizeof(capture_t))) capture_t;
            c->self = this;
            c->t1 = t1;
            c->t2 = t2;
            c->t3 = t3;

            param_ptr = (void*)c;

            // Start the thread and call the function.
            thread_id = speedyos::create_thread([](void* capture_ptr) {
                auto capture = (capture_t*)capture_ptr;
                auto func = reinterpret_cast<void (*)(T1, T2, T3)>(capture->self->function);
                func(capture->t1, capture->t2, capture->t3);
                capture->self->complete = true;

                // If thread is waiting for operation to complete, wake it up.
                if (capture->self->waiting) speedyos::awake_thread(capture->self->parent_thread_id);

                // Free parameter pointer.
                free(capture_ptr);

                return speedyos::kill_thread();
            }, c);

            return *this;
        }

        template <typename T1, typename T2, typename T3, typename T4>
        async_void& exec(T1 t1, T2 t2, T3 t3, T4 t4) {
            struct capture_t {
                async_void* self;
                T1 t1;
                T2 t2;
                T3 t3;
                T4 t4;
            };
            
            capture_t* c = new (malloc(sizeof(capture_t))) capture_t;
            c->self = this;
            c->t1 = t1;
            c->t2 = t2;
            c->t3 = t3;
            c->t4 = t4;

            param_ptr = (void*)c;

            // Start the thread and call the function.
            thread_id = speedyos::create_thread([](void* capture_ptr) {
                auto capture = (capture_t*)capture_ptr;
                auto func = reinterpret_cast<void (*)(T1, T2, T3)>(capture->self->function);
                func(capture->t1, capture->t2, capture->t3, capture->t4);
                capture->self->complete = true;

                // If thread is waiting for operation to complete, wake it up.
                if (capture->self->waiting) speedyos::awake_thread(capture->self->parent_thread_id);

                // Free parameter pointer.
                free(capture_ptr);

                return speedyos::kill_thread();
            }, c);

            return *this;
        }

        // Parks and blocks the thread until the value is resolved.
        async_void& await() {
            waiting = true;

            // Park the thread until completion.
            while (!complete) {
                speedyos::park_thread();
            }

            return *this;
        }

        // Returns whether the operation is complete.
        bool completed() {
            return complete;
        }

        ~async_void() {
            // If thread exists, kill.
            if (thread_id != 0 && !complete) speedyos::kill_thread(thread_id);
            if (param_ptr != nullptr && !complete) free(param_ptr);
        }
    private:
        bool complete = false;
        bool waiting = false;
        uint32_t thread_id = 0;
        uint32_t parent_thread_id = 0;
        void* function;
        void* param_ptr = nullptr;
};