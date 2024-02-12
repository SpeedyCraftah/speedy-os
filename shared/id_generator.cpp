#include "id_generator.h"

id_generator::id_generator(IDValidatorFunction verify_func) {
    this->validate = verify_func;
    this->prev_id = 0;
}

uint32_t id_generator::next() {
    while (this->validate(++this->prev_id));
    return this->prev_id;
}