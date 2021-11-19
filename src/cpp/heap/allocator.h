#pragma once

#include <stdint.h>

struct MARRecord {
    // 0 = kernel.
    uint32_t process_id = 0;

    uint8_t* location;
    int size;
    bool reserved;
};

extern "C" uint32_t current_process;

class heap {
    private:
        static uint32_t heap_allocated_bytes;

        static MARRecord* mar_array;
        static uint32_t MARRecordCounter;
        static uint32_t reserved_blocks;

        static uint8_t* next_alloc_addr;

        template<typename T>
        inline static uint8_t* to_reg_ptr(T* pointer) {
            return (uint8_t*)(pointer);
        }
    
    public:
        // TODO: Check if MAR is within bounds.

        static uint32_t get_allocated_bytes();
        static uint32_t get_total_blocks();
        static uint32_t get_reserved_blocks();

        // Allocates {size} of memory in the heap with an option to clear old data.
        // skip_mar should never be used.
        template<typename T>
        static T* malloc(uint32_t size = sizeof(T), bool reset = true, bool skip_mar = false, uint32_t process_id = current_process) {
            T* location = nullptr;

            // Iterate over the MAR to find any previous suitable locations.
            if (!skip_mar && MARRecordCounter != 0) {
                for (uint32_t i = 0; i < MARRecordCounter; i++) {
                    MARRecord* block = &mar_array[i];

                    // If we've reached the end of the list (left for security).
                    if (block->location == 0) break;

                    // Continue if the block is in use.
                    if (block->reserved) continue;

                    // If the block size is equal or no larger than 50 bytes of the requested size.
                    if (block->size == size || (block->size >= size && block->size - size <= 50)) {
                        // Clear the block off of old memory if requested.
                        if (reset) {
                            for (int j = 0; j <= block->size; j++) {
                                block->location[j] = 0;
                            }
                        }

                        block->reserved = true;
                        block->process_id = process_id;
                        location = (T*)block->location;

                        heap_allocated_bytes += block->size;
                        reserved_blocks++;
                    }
                }
            }

            // If we haven't found a satisfactory block or skip_mar has been requested.
            if (location == nullptr) {
                uint8_t* meta_location = next_alloc_addr;
                
                int mar_index = MARRecordCounter++;

                uint8_t* data_location = meta_location;

                // Add to MAR.
                MARRecord record;
                record.size = size;
                record.location = data_location;
                record.reserved = true;
                record.process_id = process_id;
                
                mar_array[mar_index] = record;

                next_alloc_addr = data_location + size;

                location = (T*)data_location;

                heap_allocated_bytes += size;
                reserved_blocks++;
            }

            // TODO: Return nullptr if new block cannot be allocated.

            return location;
        };

        static void free_by_process_id(uint32_t process_id) {
            for (uint32_t i = 0; i < MARRecordCounter; i++) {
                MARRecord* block = &mar_array[i];

                // If we've reached the end of the list (left for security).
                if (block->location == 0) break;

                // Continue if the block is not reserved.
                if (!block->reserved) continue;

                // If the block is not owned by the process.
                if (block->process_id != process_id) continue;

                // Found a block, deallocate.
                free(block->location);
            }
        }

        // Deallocates a memory block. 1 = Deallocated, 0 = Could not deallocate.
        template <typename T>
        static bool free(T* address) {
            for (uint32_t i = 0; i < MARRecordCounter; i++) {
                MARRecord record = mar_array[i];
                
                // Check if record location matches freeing location.
                if (record.location != (uint8_t*)address) continue;

                // If block is not reserved, stop.
                if (!record.reserved) break;

                // Block is now free and may be reused.
                record.reserved = false;

                heap_allocated_bytes -= record.size;
                reserved_blocks--;

                return true;
            }

            return false;
        }
};