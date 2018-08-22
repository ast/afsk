//
//  modem.h
//  afsk
//
//  Created by Albin Stigö on 2018-08-22.
//  Copyright © 2018 Albin Stigo. All rights reserved.
//

#ifndef modem_h
#define modem_h

#include <stdio.h>
#include <complex.h>

#include "ringbuffer.h"
#include "hdlc.h"

#define MAX_FILTER_LENGTH 128

typedef struct afsk_t {
    ringbuffer_t    rb;
    uint16_t        h_n;
    // Matched filter coefficients
    float complex   h_mark[MAX_FILTER_LENGTH];
    float complex   h_space[MAX_FILTER_LENGTH];
    // Bit recovery
    uint32_t        sym_phase_inc;
    uint8_t         shift_reg;
    uint8_t         sym_reg;
    uint32_t        sym_phase;
    // HDLC layer
    hdlc_t hdlc;
} afsk_t;

void afsk_init(afsk_t *afsk, float samplerate, float baudrate, float f_mark, float f_space);
void afsk_process(afsk_t *afsk, float *samples, uint32_t n);

#endif /* modem_h */
