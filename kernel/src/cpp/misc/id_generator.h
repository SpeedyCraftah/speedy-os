#pragma once

#include "stdint.h"

typedef bool (*IDValidatorFunction)(uint32_t);

class id_generator {
    public:
        id_generator(IDValidatorFunction verify_func);

        uint32_t next();
    
    private:
        IDValidatorFunction validate;
        uint32_t prev_id = 0;
};