#pragma once

#include "../heap/allocator.h"
#include "../misc/str.h"
#include "../panic/panic.h"
#include "../misc/algorithm.h"

namespace structures {
    template <class KeyT, class ValT>
    class map {
        public:
            struct entry {
                bool empty = true;
                unsigned int probes;
                KeyT key;
                ValT value;
            };

            map(unsigned int initialCapacity = 10) {
                capacity = initialCapacity;

                storage_ptr = heap::malloc<entry>(sizeof(entry) * initialCapacity, true);

                for (int i = 0; i < capacity; i++) {
                    storage_ptr[i] = entry();
                }
            }

            ~map() {
                heap::free(storage_ptr);
            }

            unsigned int get_capacity() {
                return capacity;
            }

            unsigned int get_occupied_buckets() {
                return occupiedBuckets;
            }

            // Unsafe raw method to getting bucket data.
            entry get_raw_bucket_at(unsigned int index) {
                if (index > capacity || index < 0) return entry();

                return storage_ptr[index];
            }

            void resize(unsigned int newCapacity) {
                // Safety checks to prevent overflowing allocated memory.
                if (newCapacity <= capacity) return;

                unsigned int oldCapacity = capacity;

                entry* old_storage_ptr = storage_ptr;
                entry* new_storage_ptr = heap::malloc<entry>(sizeof(entry) * newCapacity, false);

                // Replace current storage with new storage for easy rehashing.
                storage_ptr = new_storage_ptr;
                capacity = newCapacity;

                occupiedBuckets = 0;

                // Initialise empty storage elements.
                for (int i = 0; i < newCapacity; i++) {
                    new_storage_ptr[i] = entry();
                }

                // Table is ready to be rehashed.

                for (int i = 0; i < oldCapacity; i++) {
                    if (old_storage_ptr[i].empty) continue;

                    // Rehash the table (skip auto optimisation to prevent bugs).
                    set(old_storage_ptr[i].key, old_storage_ptr[i].value, true);
                }

                // Finally free the old table from memory.
                heap::free<entry>(old_storage_ptr);
            }

            ValT& fetch(KeyT key) {
                bucketLookupResult result = locate_bucket(key);
                if (!result.found) {
                    kernel::panic("A non-existant entry was queried. Entries must be checked for existance before fetching.");
                }

                return storage_ptr[result.index].value;
            }

            void remove(KeyT key) {
                bucketLookupResult result = locate_bucket(key);

                // Silently exit as we do not need an output.
                if (!result.found) return;

                occupiedBuckets--;

                storage_ptr[result.index] = entry();
                
                uint32_t shift_index = result.index;
                
                // Begin shifting keys back mechanism (if any).
                while (1) {
                    shift_index++;

                    // If empty, break.
                    if (storage_ptr[shift_index].probes == 0) break;
                    
                    // Needs to be moved back.
                    entry targetBucket = storage_ptr[shift_index];

                    targetBucket.probes--;
                    storage_ptr[shift_index - 1] = targetBucket;
                    
                    storage_ptr[shift_index] = entry();
                }
            }

            inline bool exists(KeyT key) {
                return locate_bucket(key).found;
            }

            // Sets a key. If optimisation is skipped and the map overflows, a kernel panic will occur.
            void set(KeyT key, ValT value, bool auto_optimise = true) {
                if (locate_bucket(key).found)
                    kernel::panic("A key has attempted to be set which would result in a duplicate entry as overwriting is disabled for safety.");

                // Resize the table for more storage and quicker access speeds.
                if (auto_optimise && (float)occupiedBuckets / capacity >= 0.92) {
                    resize((unsigned int)((float)capacity * 1.9));
                }

                if (occupiedBuckets >= capacity)
                    kernel::panic("A map value has attempted to be appended; however the maps capacity has been exhausted and a resize has not commenced.");

                uint32_t hash = hash_key(key);
                uint32_t index = hash % capacity;

                // Define new entry.
                entry target;
                target.key = key;
                target.value = value;
                target.probes = 0;
                target.empty = false;

                if (storage_ptr[index].empty) {
                    storage_ptr[index] = target;
                } else {
                    // Probe for location.
                    while (1) {
                        index++;

                        // Move onto next probe.
                        target.probes++;

                        // Boundary checks.
                        if (index > capacity - 1) index = 0;
                        else if (index < 0) index = capacity - 1;

                        entry bucket = storage_ptr[index];

                        // If a free bucket has been found.
                        if (bucket.empty) {
                            storage_ptr[index] = target;
                            break;
                        } else {
                            // Swap richer bucket for poorer.
                            if (bucket.probes < target.probes) {
                                entry oldBucket = bucket;

                                storage_ptr[index] = target;
                                target = oldBucket;
                            }
                        }
                    }
                }

                occupiedBuckets++;
            }

        private:
            entry* storage_ptr;
            unsigned int capacity;
            unsigned int occupiedBuckets = 0;

            inline unsigned int hash_key(int key) {
                return key;
            }

            struct bucketLookupResult {
                unsigned int index;
                bool found;
            };

            bucketLookupResult locate_bucket(KeyT key) {
                uint32_t hash = hash_key(key);
                uint32_t index = hash % capacity;

                // If found at allocated hash.
                if (storage_ptr[index].key == key) {
                    return bucketLookupResult({ index, true });
                }

                // Start locating by probing.
                while (1) {
                    index++;

                    // If bounds are exceeded, loop round.
                    if (index > capacity - 1) index = 0;
                    else if (index < 0) index = capacity - 1;

                    entry bucket = storage_ptr[index];

                    // Not found.
                    if (bucket.probes == 0)
                        return bucketLookupResult({ 0, false });

                    else if (bucket.key == key) {
                        return bucketLookupResult({ index, true });
                    }
                }
            }

            inline uint32_t hash_key(char* key) {
                return algorithm::hash_string_fnv1a(key);
            }
    };
};
