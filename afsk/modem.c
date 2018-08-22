//
//  modem.c
//  afsk
//
//  Created by Albin Stigö on 2018-08-22.
//  Copyright © 2018 Albin Stigo. All rights reserved.
//

#include "modem.h"

#include <math.h>
#include <complex.h>

// TODO: need to profile carefully to see if this is pointless.
static inline int
fast_is_greater(float* f1, float* f2)
{
    int i1, i2, t1, t2;
    
    i1 = *(int*)f1;
    i2 = *(int*)f2;
    
    t1 = i1 >> 31;
    i1 = (i1 ^ t1) + (t1 & 0x80000001);
    
    t2 = i2 >> 31;
    i2 = (i2 ^ t2) + (t2 & 0x80000001);
    
    return i1 > i2;
}

// Estimate the magnitude of a complex number without using transcendental functions.
// Should be faster on some platforms.
static inline float
cfastmag(float complex *in) {
    static const float alpha = 0.947543636291;
    static const float beta = 0.392485425092;
    float abs_i = fabsf(crealf(*in));
    float abs_q = fabsf(cimagf(*in));
    
    // Mag ~=Alpha * max(|I|, |Q|) + Beta * min(|I|, |Q|)
    if (fast_is_greater(&abs_i, &abs_q)) {
        return alpha * abs_i + beta * abs_q;
    } else {
        return alpha * abs_q + beta * abs_i;
    }
}

static inline float
convolve(ringbuffer_t *rb, float complex *h, uint16_t h_n) {
    float complex out = 0.;
    for (int i = 0; i < h_n; i++) {
        out += rb_read(rb, i) * h[i];
    }
    return cfastmag(&out);
    //return cabsf(out);
}

void afsk_init(afsk_t *afsk, float samplerate, float baudrate, float freq_mark, float freq_space)
{
    assert(samplerate <= 48000);
    hdlc_init(&afsk->hdlc);
    
    const float tweak = 1.25;
    afsk->h_n = roundf(tweak * samplerate / baudrate);
    
    assert(afsk->h_n < MAX_FILTER_LENGTH);
    
    // Init history buffer
    rb_init(&afsk->rb, afsk->h_n);
    // Leave space for one.
    rb_fill(&afsk->rb, 0.0);
    rb_shift(&afsk->rb);

    afsk->sym_phase_inc = ceilf(0x10000u * baudrate / samplerate);
    afsk->shift_reg = 0;
    afsk->sym_reg = 0;
    afsk->sym_phase = 0;
    
    // Setup matched filter kernels
    float phi_m, phi_s;
    int i;
    for(phi_m = phi_s = 0, i = 0; i < afsk->h_n; i++) {
        // Coeffecients
        afsk->h_mark[i] = cos(phi_m) + I * sin(phi_m);
        afsk->h_space[i] = cos(phi_s) + I * sin(phi_s);
        
        // Increment phase
        phi_m +=  2. * M_PI * (float) freq_mark / (float)samplerate;
        phi_s +=  2. * M_PI * (float) freq_space / (float)samplerate;
    }
}

void afsk_process(afsk_t *afsk, float *samples, uint32_t n)
{
    for (uint32_t i = 0; i < n; i++) {

        // Add sample to history buffer
        rb_push(&afsk->rb, samples[i]);

        // Execute correlation.
        float out = convolve(&afsk->rb, afsk->h_mark, afsk->h_n) - convolve(&afsk->rb, afsk->h_space, afsk->h_n);

        // Shift left by 1
        afsk->shift_reg <<= 1;
        afsk->shift_reg |= (out > 0);
        // Simple fixed point PLL
        // check if transition to adjust symbol phase
        if ((afsk->shift_reg ^ (afsk->shift_reg >> 1)) & 1) {
            if(afsk->sym_phase < (0x8000u-(afsk->sym_phase_inc/2))) {
                // We are a bit behind.
                afsk->sym_phase += afsk->sym_phase_inc / 8;
            } else {
                // We are a bit early.
                afsk->sym_phase -= afsk->sym_phase_inc / 8;
            }
        }
        
        // Increment symbol phase
        afsk->sym_phase += afsk->sym_phase_inc;
        if (afsk->sym_phase >= 0x10000u) {
            afsk->sym_phase &= 0xffffu; // wrap symbol phase
            
            afsk->sym_reg <<= 1;
            afsk->sym_reg |= afsk->shift_reg & 1;
            // Check if transition since last symbol?
            uint8_t bit = (afsk->sym_reg ^ (afsk->sym_reg >> 1) ^ 1) & 1;
            
            hdlc_rx_bit(&afsk->hdlc, bit);
        }
        
        // Consume sample leaving space for next sample.
        rb_shift(&afsk->rb);
    }
}
