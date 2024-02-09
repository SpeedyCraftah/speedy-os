#pragma once

#include "_shared.h"

#include "stdint.h"
#include "conversions.h"

namespace structures {

    // A special array used for stack-like storing and retrieval of elements.
    // Elements are not stored consecutively in memory and each contain an address to the next element.
    // This is designed for queue structures as finding data via an index is slow as the algorithm
    // has to travel down the element link to find the element.

    template <class T>
    class linked_array {
        public:
            linked_array(uint32_t initialSize = 10) {
                storage_ptr = (entry*)_shared_malloc(sizeof(entry) * initialSize);
                capacity = initialSize;

                // Add head and tail entries.

                // Head.
                head_entry.occupied = true;
                head_entry.next = &tail_entry;
                head_entry.prev = nullptr;

                // Tail.
                tail_entry.occupied = true;
                tail_entry.next = nullptr;
                tail_entry.prev = &head_entry;
            }

            ~linked_array() {
                // Free storage pointer.
                _shared_free(storage_ptr);
            }

            struct entry {
                bool occupied;
                T value;

                entry* next;
                entry* prev;
            };

            struct iterator {
                entry* current_entry;
                linked_array* object;
                
                void reset() {
                    current_entry = &object->head_entry;
                }
                
                // Removes the current entry in the iterator.
                void remove() {
                    // Set element before to element after.
                    current_entry->prev->next = current_entry->next;
    
                    // Set element after to element before.
                    current_entry->next->prev = current_entry->prev;
                    
                    // Mark as empty.
                    current_entry->occupied = false;
                    object->cache_index(current_entry);
                    
                    // Set current element to previous.
                    current_entry = current_entry->prev;
                    
                    object->occupied--;
                }
                
                // Shows if there is another entry in the iterator.
                inline bool hasNext() {
                    return current_entry->next != &object->tail_entry;
                }
                
                inline T& next() {
                    current_entry = current_entry->next;
                    return current_entry->value;
                }
            };
            
            iterator create_iterator() {
                iterator i;
                i.object = this;
                i.current_entry = &head_entry;
                
                return i;
            }
            
            // Adds an element to the end of the list.
            void push(const T value) {
                // Auto resize.
                if (occupied + 2 > capacity) {
                    resize(((unsigned int)((float)capacity * 2)));
                }

                // Find next available index.
                entry* index = probe_index();

                // Add and link the element.

                // Add element and link to tail and previous.
                index->value = value;
                index->occupied = true;
                index->next = &tail_entry;
                index->prev = tail_entry.prev;

                // Link tail and previous last element to new element.
                tail_entry.prev->next = index;
                tail_entry.prev = index;

                occupied++;
            }

            // Removes the last element from the list.
            T pop() {
                // Get the element.
                entry* element = tail_entry.prev;

                // Set element before to tail entry.
                element->prev->next = &tail_entry;

                // Set tail entry previous to next element.
                tail_entry.prev = element->prev;

                // Mark as empty.
                element->occupied = false;
                cache_index(element);

                occupied--;

                return element->value;
            }

            // Returns the last element without removing it.
            T& peek() {
                return tail_entry.prev->value;
            }

            // Adds an element to the front of the list.
            void unshift(const T value) {
                // Auto resize.
                if (occupied + 2 > capacity) {
                    resize(((unsigned int)((float)capacity * 1.9)));
                }

                // Find next available index.
                entry* index = probe_index();

                // Add and link the element.

                // Add element and link to head and next.
                index->value = value;
                index->occupied = true;
                index->next = head_entry.next;
                index->prev = &head_entry;

                // Link head and next first element to new element.
                head_entry.next->prev = index;
                head_entry.next = index;

                occupied++;
            }

            // Removes the first element from the list.
            T shift() {
                // Get the element.
                entry* element = head_entry.next;

                // Set head entry to point to next element.
                head_entry.next = element->next;

                // Set next elements previous to head entry.
                element->next->prev = &head_entry;

                // Mark as empty.
                element->occupied = false;
                cache_index(element);

                occupied--;

                return element->value;
            }

            // Returns the first element without removing it.
            T& peek_front() {
                return head_entry.next->value;
            }

            // Finds the index and returns the value.
            // Slow as it has to jump through many chains of elements to find the element.
            // Use arrays instead for this if applicable.
            T& get_at(uint32_t index) {
                // Safety check.
                if (index >= occupied) {
                    _shared_panic("A linked array index was out of bounds.");
                }

                // Find the element.
                entry* current = head_entry.next;
                for (uint32_t i = 0; i < index; i++) {
                    current = current->next;
                }

                return current->value;
            }

            // Remove an element at the given index.
            void remove_at(uint32_t index) {
                // Safety check.
                if (index >= occupied) {
                    _shared_panic("A linked array index was out of bounds.");
                }            

                // Find the element.
                entry* element = head_entry.next;
                for (uint32_t i = 0; i < index; i++) {
                    element = element->next;
                }

                // Set element before to element after.
                element->prev->next = element->next;

                // Set element after to element before.
                element->next->prev = element->prev;

                // Mark as empty.
                element->occupied = false;
                cache_index(element);

                occupied--;
            }

            // May be improved.
            void resize(uint32_t newCapacity) {
                if (newCapacity < occupied) {
                    _shared_panic("A linked array was resized to a smaller capacity.");
                }

                // Create a new storage area.
                entry* new_storage_ptr = new entry[newCapacity];

                // Move data from old area to new area.
                entry* current = head_entry.next;
                uint32_t index = 0;

                while (current != &tail_entry) {
                    new_storage_ptr[index].value = current->value;
                    new_storage_ptr[index].occupied = true;

                    if (current->prev == &head_entry) {
                        new_storage_ptr[index].prev = &head_entry;
                    } else {
                        new_storage_ptr[index].prev = &new_storage_ptr[index - 1];
                    }

                    if (current->next == &tail_entry) {
                        new_storage_ptr[index].next = &tail_entry;
                    } else {
                        new_storage_ptr[index].next = &new_storage_ptr[index + 1];
                    }

                    current = current->next;
                    index++;
                }

                // Update head and tail entry.
                head_entry.next = &new_storage_ptr[0];
                tail_entry.prev = &new_storage_ptr[index - 1];

                // Clear caches.
                next_index = nullptr;
                next_index_2 = nullptr;

                // Free old area.
                _shared_free(storage_ptr);

                capacity = newCapacity;
                storage_ptr = new_storage_ptr;
            }

            uint32_t get_capacity() {
                return capacity;
            }

            uint32_t get_size() {
                return occupied;
            }

        private:
            void cache_index(entry* index) {
                if (next_index == nullptr) {
                    next_index = index;
                } else if (next_index_2 == nullptr) {
                    next_index_2 = index;
                }
            }

            entry* probe_index() {
                // Exhaust cached indexes.
                if (next_index != nullptr) {
                    entry* index = next_index;
                    next_index = nullptr;
                    return index;
                }

                if (next_index_2 != nullptr) {
                    entry* index = next_index_2;
                    next_index_2 = nullptr;
                    return index;
                }

                // Find an empty slot.
                for (uint32_t i = 0; i < capacity; i++) {
                    entry e = storage_ptr[i];
                    if (!e.occupied) {
                        return &storage_ptr[i];
                    }
                }
                
                _shared_panic("A linked array was full.");
                __builtin_unreachable();
            }

            entry head_entry;
            entry tail_entry;

            entry* storage_ptr;

            uint32_t occupied = 0;
            uint32_t capacity = 0;

            entry* iterator_last_entry;

            // A cache of indexes for blocks (will be improved).
            entry* next_index = nullptr;
            entry* next_index_2 = nullptr;
    };
}