#pragma once

#include "stdint.h"

#include "structures/thread.h"
#include "structures/events.h"

namespace scheduler {
  namespace events {
    void emit_event(Process* sender_process, uint32_t id, uint32_t data);
  }
}