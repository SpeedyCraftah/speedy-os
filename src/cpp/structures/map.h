#pragma once

#include "../heap/allocator.h"
#include "../misc/str.h"
#include "../panic/panic.h"

namespace structures {
    template <class KeyT, class ValT>
    class map {
        public:
            map(unsigned int initialCapacity = 10) {
                capacity = initialCapacity;
                storage_ptr = heap::malloc<entry>(sizeof(entry) * initialCapacity, false);

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
                uint32_t hash = hash_key(key);
                unsigned int index = hash % capacity;

                if (!storage_ptr[index].empty && storage_ptr[index].key == key) {
                    return storage_ptr[index].value;
                } else {
                    int total = 0;
                    int i = index + 1;

                    while (1) {
                        //if (total >= occupiedBuckets) return nullptr;
                        
                        if (i > capacity - 1) i = 0;
                        else if (i < 0) i = capacity - 1;

                        if (!storage_ptr[i].empty && storage_ptr[i].key == key) {
                            return storage_ptr[i].value;
                        }

                        i++;
                    }
                }
            }

            void remove(KeyT key) {
                uint32_t hash = hash_key(key);
                unsigned int index = hash % capacity;

                if (!storage_ptr[index].empty && storage_ptr[index].key == key) {
                    // No need to clear all elements. Will be overwritten anyways.
                    storage_ptr[index].empty = true;

                    return;
                } else {
                    int total = 0;
                    int i = index + 1;

                    while (1) {
                        if (total >= occupiedBuckets) return;

                        if (i > capacity - 1) i = 0;
                        else if (i < 0) i = capacity - 1;

                        if (!storage_ptr[i].empty && storage_ptr[i].key == key) {
                            storage_ptr[i].empty = true;
                            break;
                        }

                        i++;
                        total++;
                    }
                }

                occupiedBuckets--;
            }

            bool exists(KeyT key) {
                uint32_t hash = hash_key(key);
                unsigned int index = hash % capacity;

                if (!storage_ptr[index].empty && storage_ptr[index].key == key) {
                    return true;
                } else {
                    int total = 0;
                    int i = index + 1;

                    while (1) {
                        if (total >= occupiedBuckets) break;
                        
                        if (i > capacity - 1) i = 0;
                        else if (i < 0) i = capacity - 1;

                        if (!storage_ptr[i].empty && storage_ptr[i].key == key) {
                            return storage_ptr[i].value;
                        }

                        i++;
                    }
                }

                return false;
            }

            // Sets a key. If optimisation is skipped and the map overflows, a kernel panic will occur.
            void set(KeyT key, ValT& value, bool auto_optimise = true) {
                // Resize the table for more storage and quicker access speeds.
                if (auto_optimise && (float)occupiedBuckets / capacity >= 0.92) {
                    resize((unsigned int)((float)capacity * 1.9));
                }

                if (occupiedBuckets >= capacity) kernel::panic("A map value has attempted to be appended; however the maps capacity has been exhausted and a resize has not commenced.");

                uint32_t hash = hash_key(key);
                unsigned int index = hash % capacity;

                entry target;
                target.key = key;
                target.value = value;
                target.probes = 0;
                target.empty = false;

                if (storage_ptr[index].empty) {
                    storage_ptr[index] = target;
                } else {
                    int i = index;

                    while (1) {
                        if (i > capacity - 1) i = 0;
                        else if (i < 0) i = capacity - 1;

                        entry bucket = storage_ptr[i];

                        if (bucket.empty) {
                            storage_ptr[i] = target;
                            break;
                        } else {
                            if (bucket.probes < target.probes) {
                                entry oldBucket = bucket;
                                storage_ptr[i] = target;

                                target = oldBucket;

                                // Check if old position is available by any chance.
                                int newBucketHashIndex = hash_key(target.key) % capacity;

                                if (i != newBucketHashIndex && storage_ptr[newBucketHashIndex].empty) {
                                    target.probes = 0;
                                    storage_ptr[newBucketHashIndex] = target;

                                    break;
                                }
                            }
                        }

                        i++;
                        target.probes++;
                    }
                }

                occupiedBuckets++;
            }

        private:
            struct entry {
                bool empty = true;
                unsigned int probes;
                KeyT key;
                ValT value;
            };

            entry* storage_ptr;
            unsigned int capacity;
            unsigned int occupiedBuckets = 0;

            inline unsigned int hash_key(int key) {
                return key;
            }

            uint32_t hash_key(char* key) {
                uint32_t fnvPrime = 16777619;
                uint32_t hash = 2166136261;

                int i = 0;

                while (1) {
                    if (key[i] == '\0') break;

                    int characterCode = (int)key[i];

                    hash ^= characterCode;
                    hash *= fnvPrime;

                    i++;
                }

                return hash;
            }
    };
};