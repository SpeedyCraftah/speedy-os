#pragma once

#include "../misc/id_generator.h"
#include "scheduler.h"
#include "structures/datasink.h"

namespace scheduler {
    namespace datasink {
        extern id_generator id_gen;
        extern structures::map<SteadyDataSink*> active_sinks;

        void initialise();
        SteadyDataSink* create_steady_datasink(Process* process);
    };
};