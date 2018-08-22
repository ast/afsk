//
//  hdlc.h
//  afsk
//
//  Created by Albin Stigö on 09/06/2017.
//  Copyright © 2017 Albin Stigo. All rights reserved.
//

#ifndef hdlc_h
#define hdlc_h

#include <stdio.h>

#include <stdbool.h>
#include <stdint.h>

#define MAX_PACKET_LEN 256

//static const int max_packet_length  = 256;
static const uint8_t hdl_frame_flag = 0x7e;

typedef struct hdlc_t {
    uint8_t     bitstream;
    uint16_t    bitbuf;
    uint8_t     rx_buf[MAX_PACKET_LEN];
    uint8_t     *rx_ptr;
    bool        rx_state;
    // statistics
    // total packets decoded
    uint32_t    n_packets;
} hdlc_t;

void hdlc_init(hdlc_t *hdlc);
void hdlc_rx_bit(hdlc_t *hdlc, uint8_t bit);

#endif /* hdlc_h */
