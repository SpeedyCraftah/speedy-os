#include "datasink.h"
#include "structures/datasink.h"
#include "../misc/assert.h"

namespace scheduler {
    namespace datasink {
        id_generator id_gen(nullptr);
        structures::map<SteadyDataSink*> active_sinks;

        void initialise() {
            // Cannot trust C++ with statically initialising id generator.
            id_gen = id_generator([](uint32_t id) -> bool { return scheduler::datasink::active_sinks.exists(id); });
            active_sinks = structures::map<SteadyDataSink*>();
        }

        SteadyDataSink* create_steady_datasink(Process* process) {
            SteadyDataSink* sink = new SteadyDataSink;
            sink->handle_id = id_gen.next();

            assert_eq("sch.structs.sdatasink.fragments", sink->fragments.get_capacity(), (uint32_t)10);

            process->steady_sinks->set(sink->handle_id, sink);
            active_sinks.set(sink->handle_id, sink);
            return sink;
        }
    }
}