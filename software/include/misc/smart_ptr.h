#pragma once

#include "../alloc.h"


template <class T>
class smart_ptr {
    public:
        smart_ptr(T* ptr) {
            storage_ptr = ptr;
        }

        ~smart_ptr() {
            if (dealloc) free(storage_ptr);
        }

        // Overload casting operator.
        operator T*() {
            return storage_ptr;
        }

        // Overload [] operator.
        T& operator[](int index) {
            return storage_ptr[index];
        }

        // Overload ++ operator.
        smart_ptr<T>& operator++() {
            storage_ptr++;
        }

        // Overload -- operator.
        smart_ptr<T>& operator--() {
            storage_ptr--;
        }

        // Overload * operator.
        T& operator*() {
            return *storage_ptr;
        }

        // Overload -> operator.
        T* operator->() {
            return storage_ptr;
        }

        // Overload delete operator.
        void operator delete(void* ptr) {
            free(ptr);
        }

        // Returns the raw pointer.
        T* ptr() {
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