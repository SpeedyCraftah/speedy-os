// A JavaScript/C# inspired event handler & listener.
// Optimised for performance, not memory.

#pragma once

#include "stdint.h"
#include "map.h"
#include "flex_array.h"
#include "../heap/allocator.h"

namespace structures {
    template <class EventT, class DataT>
    class event_handler {
        public:
            struct event_data {
                DataT data;
            };

            event_handler() {
                map<EventT, flexible_array<void(*)()>*> arr = map<EventT, flexible_array<void(*)()>*>();
                storage_ptr = arr;
            }

            ~event_handler() {
                // Todo - Improve destructor performance.
                for (unsigned int i = 0; i < storage_ptr.get_capacity(); i++) {
                    auto bucket = storage_ptr.get_raw_bucket_at(i);
                    if (bucket.empty) continue;
                    
                    heap::free(bucket.value);
                }
            }

            void add_event_listener(EventT event, void(*listener)()) {
                if (!storage_ptr.exists(event)) {
                    flexible_array<void(*)()>* arr = new flexible_array<void(*)()>();
                    storage_ptr.set(event, arr);
                }

                flexible_array<void(*)()>* arr = storage_ptr.fetch(event);
                arr->push(listener);
            }

            void remove_event_listener(EventT event, void(*listener)(), bool remove_all = false) {
                if (!storage_ptr.exists(event)) return;

                flexible_array<void(*)()>* arr = storage_ptr.fetch(event);

                for (unsigned int i = 0; i < arr->get_size(); i++) {
                    // Compare memory locations.
                    if (!arr->is_empty_at(i) && arr->get_at(i) != listener) continue;

                    // Remove from index.
                    arr->remove_at(i);

                    if (!remove_all) break;
                }

                // Defragment array.
                arr->defragment();
            }

            void emit_event(EventT event, DataT data) {
                if (!storage_ptr.exists(event)) return;
                flexible_array<void(*)()>* arr = storage_ptr.fetch(event);

                // Prepare data.
                event_data d;
                d.data = data;
                ready_event_data = d;

                for (unsigned int i = 0; i < arr->get_size(); i++) {
                    auto event_process = arr->get_at(i);
                    event_process();
                }
            }

            event_data fetch_event_data() {
                return ready_event_data;
            }

        private:
            map<EventT, flexible_array<void(*)()>*> storage_ptr;

            event_data ready_event_data;
    };
}