#pragma once

#include "_shared.h"

template <class T>
class smart_ptr {
    public:
        smart_ptr(T* ptr) {
            storage_ptr = ptr;
        }

        ~smart_ptr() {
            if (dealloc) _shared_free(storage_ptr);
        }

        // Overload casting operator.
        inline operator T*() {
            return storage_ptr;
        }

        // Overload [] operator.
        inline T& operator[](int index) {
            return storage_ptr[index];
        }

        // Overload ++ operator.
        inline smart_ptr<T>& operator++() {
            storage_ptr++;
        }

        // Overload -- operator.
        inline smart_ptr<T>& operator--() {
            storage_ptr--;
        }

        // Overload * operator.
        inline T& operator*() {
            return *storage_ptr;
        }

        // Overload -> operator.
        inline T* operator->() {
            return storage_ptr;
        }

        // Overload delete operator.
        inline void operator delete(void* ptr) {
            _shared_free(ptr);
        }

        // Returns the raw pointer.
        inline T* ptr() {
            return storage_ptr;
        }

        // Tells the smart pointer to not deallocate the pointer automatically.
        T* norm() {
            dealloc = false;

            return storage_ptr;
        }

    private:
        bool dealloc = true;

        T* storage_ptr;
};