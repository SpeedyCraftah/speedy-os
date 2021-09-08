#include "../process_t.h"
#include "../../cpp/io/video.h"

namespace Processes {
    class System : public Software::ProcessRuntime {
        public:
            void on_start() {
                video::printf("\nSYSTEM: Started.\n");

                while (true) {
                    ticks++;

                    if (ticks % 100000 == 0) video::printf("\nSystem says HI!");

                    yield();
                }
            }

            void on_exit() {
                
            }

        private:
            int ticks = 0;
    };
}