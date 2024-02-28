#pragma once

#include "_shared.h"

#include "str.h"
#include "algorithm.h"

namespace structures {
    template <class ValT>
    class map {
        public:
            struct entry {
                bool empty = true;
                uint32_t probes;
                uint32_t key;
                ValT value;
            };

            map(uint32_t initialCapacity = 10) {
                capacity = initialCapacity;

                storage_ptr = (entry*)_shared_malloc(sizeof(entry) * initialCapacity, true);

                for (int i = 0; i < capacity; i++) {
                    storage_ptr[i] = entry();
                }
            }

            ~map() {
                _shared_free(storage_ptr);
            }

            uint32_t get_capacity() {
                return capacity;
            }

            uint32_t get_occupied_buckets() {
                return occupiedBuckets;
            }

            void resize(uint32_t newCapacity) {
                // Safety checks to prevent overflowing allocated memory.
                if (newCapacity <= capacity) return;

                uint32_t oldCapacity = capacity;

                entry* old_storage_ptr = storage_ptr;
                entry* new_storage_ptr = (entry*)_shared_malloc(sizeof(entry) * newCapacity, false);

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
                _shared_free(old_storage_ptr);
            }

            ValT& fetch(uint32_t key) {
                bucketLookupResult result = locate_bucket(key);
                if (!result.found) {
                    _shared_panic("A non-existant entry was queried. Entries must be checked for existance before fetching.");
                }

                return storage_ptr[result.index].value;
            }

            ValT& fetch(char* key) {
                return fetch(hash_key(key));
            }

            void remove(uint32_t key) {
                bucketLookupResult result = locate_bucket(key);

                // Silently exit as we do not need an output.
                if (!result.found) return;

                occupiedBuckets--;

                storage_ptr[result.index] = entry();
                
                uint32_t shift_index = result.index;
                
                // Begin shifting keys back mechanism (if any).
                while (1) {
                    shift_index++;

                    // Boundary checks.
                    if (shift_index > capacity - 1) shift_index = 0;

                    // If empty, break.
                    if (storage_ptr[shift_index].probes == 0) break;
                    
                    // Needs to be moved back.
                    entry targetBucket = storage_ptr[shift_index];

                    targetBucket.probes--;
                    storage_ptr[shift_index - 1] = targetBucket;
                    
                    storage_ptr[shift_index] = entry();
                }
            }

            inline void remove(char* key) {
                remove(hash_key(key));
            }

            inline bool exists(uint32_t key) {
                return locate_bucket(key).found;
            }

            inline bool exists(char* key) {
                return exists(hash_key(key));
            }

            // Sets a key. If optimisation is skipped and the map overflows, a kernel panic will occur.
            void set(uint32_t key, ValT value, bool auto_optimise = true) {
                if (locate_bucket(key).found)
                    _shared_panic("A key has attempted to be set which would result in a duplicate entry as overwriting is disabled for safety.");

                // Resize the table for more storage and quicker access speeds.
                if (auto_optimise && (float)occupiedBuckets / capacity >= 0.92) {
                    resize((unsigned int)((float)capacity * 1.9));
                }

                if (occupiedBuckets >= capacity)
                    _shared_panic("A map value has attempted to be appended; however the maps capacity has been exhausted and a resize has not commenced.");

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

            inline void set(char* key, ValT value, bool auto_optimise = true) {
                return set(hash_key(key), value, auto_optimise);
            }

            struct iterator {
                class MapIteratorElement {
                    public:
                        inline MapIteratorElement(map::entry* e) : entry(e) {};
                        
                        inline uint32_t key() {
                            return entry->key;
                        }
                        inline ValT& value() {
                            return entry->value;
                        }

                    private:
                        map::entry* entry;
                };

                uint32_t current_entry = 0;
                map* object;
                
                void reset() {
                    current_entry = 0;
                }
                
                // Shows if there is another entry in the iterator.
                inline bool hasNext() {
                    while (current_entry != object->capacity) {
                        entry& e = object->storage_ptr[current_entry];
                        if (!e.empty) return true;
                        else current_entry++;
                    }

                    return false;
                }
                
                inline MapIteratorElement next() {
                    return MapIteratorElement(&object->storage_ptr[current_entry++]);
                }
            };
            
            iterator create_iterator() {
                iterator i;
                i.object = this;
                
                return i;
            }

        private:
            entry* storage_ptr;
            uint32_t capacity;
            uint32_t occupiedBuckets = 0;

            inline uint32_t hash_key(int key) {
                return key;
            }

            inline uint32_t hash_key(char* key) {
                return algorithm::hash_string_fnv1a(key);
            }

            struct bucketLookupResult {
                unsigned int index;
                bool found;
            };

            bucketLookupResult locate_bucket(uint32_t key) {
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
                    
                    entry bucket = storage_ptr[index];

                    // Not found.
                    if (bucket.probes == 0)
                        return bucketLookupResult({ 0, false });

                    else if (bucket.key == key) {
                        return bucketLookupResult({ index, true });
                    }
                }
            }

            inline bucketLookupResult locate_bucket(char* key) {
                return locate_bucket(hash_key(key));
            }
    };
};
