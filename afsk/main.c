//
//  main.c
//  afsk
//
//  Created by Albin Stigö on 09/06/2017.
//  Copyright © 2017 Albin Stigo. All rights reserved.
//

#include <stdio.h>
#include <sndfile.h>
#include <stdlib.h>
#include <complex.h>
#include <math.h>

#include "ringbuffer.h"

#include "hdlc.h"

static const int samplerate     = 24000;
static const int baudrate       = 1200;
static const int freq_mark      = 1200;
static const int freq_space     = 2200;

static const int corr_len       = samplerate / baudrate;

static float complex corr_mark[corr_len];
static float complex corr_space[corr_len];

static const uint16_t sym_phase_inc = 0x10000u * baudrate / samplerate;

#define ARRAY_SIZE(array) \
(sizeof(array) / sizeof(*array))

float correlate(ringbuffer_t *rb, float complex corr[static corr_len]) {
    float complex out = 0.;
    for (int i = 0; i < corr_len; i++) {
        out += rb_read(rb, i) * corr[i];
    }
    return cabsf(out);
}

int main(int argc, const char * argv[]) {

    // Init ringbuffer
    ringbuffer_t rb;
    rb_init(&rb, corr_len);
    // Leave space for one.
    rb_fill(&rb, 0.0);
    rb_shift(&rb);
    
    // Setup matched filters
    float phi_m, phi_s;
    int i;
    for(phi_m = phi_s = 0, i = 0; i < corr_len; i++) {
        // Coeffecients
        corr_mark[i] = cos(phi_m) + I * sin(phi_m);
        corr_space[i] = cos(phi_s) + I * sin(phi_s);
        // Increment phase
        phi_m +=  2. * M_PI * freq_mark / samplerate;
        phi_s +=  2. * M_PI * freq_space / samplerate;
    }
    
    // Bit recovery
    uint8_t shift_reg = 0;
    uint8_t sym_reg = 0;
    uint32_t sym_phase = 0;
    
    // HDLC
    hdlc_t hdlc;
    hdlc_init(&hdlc);
    
    SF_INFO si;
    SNDFILE* sf;

    
    if (argc < 2) {
        fprintf(stderr, "Usage: afsk wavefile\n");
        fprintf(stderr, "\thas to be mono 24000 samples/s.");
        exit(EXIT_FAILURE);
    }
    
    if((sf = sf_open(argv[1], SFM_READ, &si)) == NULL) {
        fprintf(stderr, "oops: %s\n", sf_strerror(NULL));
        exit(EXIT_FAILURE);
    }
    
    float frames[1024];
    sf_count_t n;

    while((n = sf_read_float(sf, frames, ARRAY_SIZE(frames))) > 0) {
        //printf("%ld\n", n);
        // each sample
        for (int i = 0; i < n; i++) {
            // Push new sample.
            rb_push(&rb, frames[i]);
            
            // Execute correlation.
            float out = correlate(&rb, corr_mark) - correlate(&rb, corr_space);
            //printf("%f ", out);
            
            // Shift left by 1
            shift_reg <<= 1;
            shift_reg |= (out > 0);

            // Check if transition to adjust symbol phase
            if ((shift_reg ^ (shift_reg >> 1)) & 1) {
                if(sym_phase < (0x8000u-(sym_phase_inc/2))) {
                    // We are a bit behind.
                    sym_phase += sym_phase_inc/8;
                } else {
                    // We are a bit early.
                    sym_phase -= sym_phase_inc/8;
                }
            }
            
            // Increment symbol phase
            sym_phase += sym_phase_inc;
            if (sym_phase >= 0x10000u) {
                sym_phase &= 0xffffu; // wrap symbol phase
                
                sym_reg <<= 1;
                sym_reg |= shift_reg & 1;
                // Check if transition since last symbol?
                uint8_t bit = (sym_reg ^ (sym_reg >> 1) ^ 1) & 1;

                //printf("%d", bit);
                hdlc_rx_bit(&hdlc, bit);
            }
            
            // Consume sample leaving space for next sample.
            rb_shift(&rb);
        }
    }

    sf_close(sf);
    
    return 0;
}
