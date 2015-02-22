#pragma once

static inline void quad(float* v) {
    short field = 0x0D92; // encode sign
    for(int i = 0; i < 12; ++i) {
        v[i] = (field & 1) == 0 ? -1.0 : 1.0;
        field >>= 1;
    }
}
