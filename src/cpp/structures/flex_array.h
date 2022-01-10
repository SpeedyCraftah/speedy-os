#pragma once

#include "../heap/allocator.h"
#include "../panic/panic.h"

namespace structures {

    // An array which allows the user to determine if an index is empty or not.
    // Provides useful features which are optimised for performance, not memory.

    template <class T>
    class flexible_array {
        public:
            flexible_array(unsigned int initialSize = 10, bool dealloc_entries = false) {
                if (initialSize > 2147483646) {
                    kernel::panic("A flexible array has been created with a size which exhausts a signed integer.");
                }
    
                storage_ptr = (entry*)heap::malloc(sizeof(entry) * initialSize);
                capacity = initialSize;
                this->dealloc_entries = dealloc_entries;

                for (int i = 0; i < capacity; i++) {
                    storage_ptr[i] = entry();
                }
            }

            ~flexible_array() {
                // If dealloc entries is enabled.
                // Attempts to deallocate all entries which are expected to be pointers
                // upon array being disposed.
                if (dealloc_entries) {
                    for (uint32_t i = 0; i < capacity; i++) {
                        entry* e = &storage_ptr[i];
                        if (e->empty) continue;

                        // Deallocate.
                        // The syntax is unusual since it bypasses
                        // C++ compile-time safety checks which
                        // are over-sensitive due to templating.
                        heap::free(*((void**)(&e->value)));
                    }
                }

                heap::free(storage_ptr);
            }

            void resize(unsigned int newCapacity, bool unsafe_resize = false) {
                if (newCapacity > 2147483646) {
                    kernel::panic("A flexible array has attempted to be resized with a size which exhausts a signed integer.");
                }

                if (fragmented && !unsafe_resize && newCapacity < capacity) {
                    kernel::panic("A flexible array has attempted to resize to a smaller size which triggered a protection fault as it is fragmented.");
                }

                if (!fragmented && !unsafe_resize && newCapacity < occupied) {
                    kernel::panic("A flexible array has attempted to resize to a smaller size which would exhaust the storage container.");
                }

                // Create a new storage area.
                entry* new_storage_ptr = (entry*)heap::malloc(newCapacity * sizeof(entry), false);

                // Move data from old area to new area.
                for (int i = 0; i < capacity; i++) {
                    new_storage_ptr[i] = storage_ptr[i];
                }

                // Free old area.
                heap::free(storage_ptr);

                capacity = newCapacity;
                storage_ptr = new_storage_ptr;
            }

            // Warning - Modifies indexes!
            // Moves all entries closer to the start as possible.
            void defragment() {
                int emptyIndex = -1;

                for (int i = 0; i < capacity; i++) {
                    if (storage_ptr[i].empty) {
                        if (emptyIndex == -1) emptyIndex = i;
                        continue;
                    }

                    if (emptyIndex == -1) continue;

                    // Element needs to be moved back.
                    storage_ptr[emptyIndex] = storage_ptr[i];

                    // Free up old slot.
                    storage_ptr[i] = entry();

                    i = emptyIndex;
                    emptyIndex = -1;
                }

                next_index = occupied;
                fragmented = false;
            }

            void optimise() {
                if (fragmented) defragment();
            }

            inline bool is_fragmented() {
                return fragmented;
            }

            inline bool is_empty_at(unsigned int index) {
                if (index + 1 > capacity) return false;

                return storage_ptr[index].empty;
            }

            // Alias for array[index].
            T& get_at(unsigned int index) {
                return operator[](index);
            }

            void set_at(unsigned int index, T value) {
                // Resize on the fly. (shouldn't be used).
                if (index + 1 > capacity) {
                    resize(capacity + ((index + 1) - capacity) + ((unsigned int)((float)capacity * 1.9)));
                }

                if (storage_ptr[index].empty) {
                    if (index != next_index) fragmented = true;

                    occupied++;
                    next_index++;
                }

                storage_ptr[index].empty = false;
                storage_ptr[index].value = value;
            }

            void remove_at(unsigned int index) {
                if (index + 1 > capacity || storage_ptr[index].empty) return;
                if (index != next_index - 1) {
                    fragmented = true;
                }
                    
                next_index--;
                occupied--;

                storage_ptr[index].empty = true;
            }

            void push(const T value) {
                if (fragmented) {
                    kernel::panic("A defragmented-only operation has been attempted on a flexible array despite being fragmented.");
                }

                // Auto resize.
                if (next_index + 1 > capacity) {
                    resize(capacity + ((next_index + 1) - capacity) + ((unsigned int)((float)capacity * 1.9)));
                }

                storage_ptr[next_index].value = value;
                storage_ptr[next_index].empty = false;

                occupied++;
                next_index++;
            }

            // Pops the last element and returns it.
            T pop(uint32_t amount = 1) {
                if (fragmented) {
                    kernel::panic("A defragmented-only operation has been attempted on a flexible array despite being fragmented.");
                }

                entry element;

                for (uint32_t i = 0; i < amount; i++) {
                    element = storage_ptr[next_index - 1];
                    if (element.empty) continue;
                    
                    storage_ptr[next_index - 1].empty = true;

                    occupied--;
                    next_index--;
                }

                return element.value;
            }

            // Pops the first element and returns it.
            // Inefficient as it has to defragment the whole array afterwards to maintain linear structure.
            T shift(uint32_t amount = 1) {
                if (fragmented) {
                    kernel::panic("A defragmented-only operation has been attempted on a flexible array despite being fragmented.");
                }

                entry element;

                for (uint32_t i = 0; i < amount; i++) {
                    element = storage_ptr[i];
                    if (element.empty) continue;

                    storage_ptr[0].empty = true;
                    occupied--;
                }

                defragment();

                return element.value;
            }

            // Resets the whole array back to default.
            void reset() {
                for (int i = 0; i < capacity; i++) {
                    storage_ptr[i].empty = true;
                }

                occupied = 0;
                next_index = 0;
                fragmented = false;
            }

            // Override [] operator.
            T& operator[](unsigned int index) {
                if (capacity < index + 1) kernel::panic("An operation has been attempted on a flexible array which exhausts the capacity ranges.");
                if (is_empty_at(index)) {
                    kernel::panic("An operation has been attempted on a flexible array empty bucket.");
                }

                return storage_ptr[index].value;
            }

            unsigned int get_capacity() {
                return capacity;
            }

            // Returns amount of entries which are classed as occupied.
            unsigned int get_size() {
                return occupied;
            }

        private:
            struct entry {
                bool empty = true;
                T value;
            };

            entry* storage_ptr;

            unsigned int occupied = 0;
            unsigned int capacity = 0;

            unsigned int next_index = 0;

            bool dealloc_entries = false;

            bool fragmented = false;
    };
}