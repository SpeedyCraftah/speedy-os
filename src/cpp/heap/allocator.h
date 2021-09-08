#pragma once

#include "../misc/conversions.h"
#include <stdint.h>

struct MARRecord {
    MARRecord(uint8_t* location_, int size_, bool reserved_);

    // Used to determine if a record actually exists.
    uint8_t magic_number = 174;

    uint32_t process_id;

    uint8_t* location;
    int size;
    bool reserved;
};

class heap {
    private:
        static unsigned int heap_allocated_bytes;

        static MARRecord* mar_array;
        static int MARRecordCounter;

        static uint8_t* next_alloc_addr;

        template<typename T>
        inline static uint8_t* to_reg_ptr(T* pointer) {
            return (uint8_t*)(pointer);
        }
    
    public:
        // TODO: Check if MAR is within bounds.

        // Allocates {size} of memory in the heap with an option to clear old data.
        // skip_mar should never be used.
        template<typename T>
        static T* malloc(int size = sizeof(T), bool reset = true, bool skip_mar = false) {
            T* location = nullptr;

            // Iterate over the MAR to find any previous suitable locations.
            if (!skip_mar && MARRecordCounter != 0) {
                for (int i = 0; i < MARRecordCounter; i++) {
                    MARRecord* block = &mar_array[i];

                    // If we've reached the end of the list (left for security).
                    if (block->location == 0) break;

                    // Continue if the block is in use.
                    if (block->reserved) continue;

                    // If the block size is equal or no larger than 10 bytes of the requested size.
                    if (block->size >= size && block->size - size <= 10) {
                        // Clear the block off of old memory if requested.
                        if (reset) {
                            for (int j = 0; j <= block->size; j++) {
                                block->location[j] = 0;
                            }
                        }

                        block->reserved = true;
                        location = (T*)block->location;
                    }
                }
            }

            // If we haven't found a satisfactory block or skip_mar has been requested.
            if (location == nullptr) {
                uint8_t* meta_location = next_alloc_addr;
                
                int mar_index = MARRecordCounter++;

                uint8_t* data_location = meta_location + sizeof(int);

                // Add to MAR.
                mar_array[mar_index] = MARRecord(data_location, size, true);

                *((int*)meta_location) = mar_index;

                next_alloc_addr = data_location + size;

                location = (T*)data_location;

                heap_allocated_bytes += size + sizeof(int);
            }

            // TODO: Return nullptr if new block cannot be allocated.

            return location;
        };

        // Deallocates a memory block. 1 = Deallocated, 0 = Could not deallocate.
        template <typename T>
        static bool free(T* address) {
            int mar_reference = *((int*)(to_reg_ptr(address) - sizeof(int)));
            MARRecord* record = &mar_array[mar_reference];

            // Check if record exists.
            if (record->magic_number != 174) return false;

            // Block is now free and may be reused.
            record->reserved = false;

            heap_allocated_bytes -= record->size;

            return true;
        }
};