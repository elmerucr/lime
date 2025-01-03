// video controller

#ifndef VC_HPP
#define VC_HPP

#include <cstdint>
#include "font_cbm_8x8.hpp"

class vc_t {
private:
    uint8_t *ram;

    font_cbm_8x8_t font;
    
public:
    vc_t();
    ~vc_t();
};

#endif
