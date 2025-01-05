// ---------------------------------------------------------------------
// 0x0000-0x00ff base page
// 0x0100-0x01ff system stack
// 0x0200-0x03ff io
// 0x0400-0x07ff sprite table (128 x 8 bytes), xpos, ypos, flags, pointer, palette
// 0x2000-0x37ff tilesets (6kb), 0x2000 start of set 0, 0x2800 start of set 1
// ---------------------------------------------------------------------

#ifndef SYSTEM_HPP
#define SYSTEM_HPP

class host_t;
class vdc_t;

class system_t {
private:
public:
    system_t();
    ~system_t();

    host_t *host;
    vdc_t *vdc;

    bool running;

    void run();
};

#endif
