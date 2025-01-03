#ifndef VC_HPP
#define VC_HPP

#include <cstdint>

class vc_t {
private:
    uint8_t *ram;
public:
    vc_t();
    ~vc_t();
};

#endif
