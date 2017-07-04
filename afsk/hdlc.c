//
//  hdlc.c
//  afsk
//
//  Created by Albin Stigö on 09/06/2017.
//  Copyright © 2017 Albin Stigo. All rights reserved.
//

#include "hdlc.h"

#include <unistd.h>
#include <string.h>
#include <assert.h>

#include "crc.h"

void hdlc_init(hdlc_t *hdlc) {
    hdlc->bitstream = 0x00;
    hdlc->bitbuf = 0x00;
    hdlc->rx_state = false;
    hdlc->rx_ptr = hdlc->rx_buf;
    memset(hdlc->rx_buf, 0x00, sizeof(hdlc->rx_buf));

}

void write_erl_packet(uint8_t *bytes, uint16_t len) {
    size_t written;
    uint8_t li;

    li = (len >> 8) & 0xff;
    written = write(1, &li, 1);
    assert(written == 1);

    li = len & 0xff;
    written = write(1, &li, 1);
    assert(written == 1);

    written = write(1, bytes, len);
    assert(written == len);
    
    sleep(1);
}


void hdlc_rx_bit(hdlc_t *hdlc, uint8_t bit) {
    // New bit arriving. Shift left by one.
    //Double negation is guaranteed to return 0/1.
    hdlc->bitstream <<= 1;
    hdlc->bitstream |= !!bit;
    
    // Frame flag
    // 0b01111110
    if((hdlc->bitstream & 0xff) == hdl_frame_flag) {
        
        if (hdlc->rx_state &&
            (hdlc->rx_ptr - hdlc->rx_buf) > 2) {

            if(check_crc_ccitt(hdlc->rx_buf, (int) (hdlc->rx_ptr - hdlc->rx_buf))) {
                //printf("checksum ok.");
                write_erl_packet(hdlc->rx_buf, hdlc->rx_ptr - hdlc->rx_buf);
            }
        }
        
        hdlc->rx_state = true;
        hdlc->rx_ptr = hdlc->rx_buf;
        hdlc->bitbuf = 0x80;
        return;
    }
    
    // Invalid sequence because it's not bit stuffed.
    // 0b01111111
    if ((hdlc->bitstream & 0x7f) == 0x7f) {
        hdlc->rx_state = false;
        return;
    }
    
    // If not in rx_state don't continue.
    if(!hdlc->rx_state) return;
    
    // Stuffed bit
    if((hdlc->bitstream & 0x3f) == 0x3e) return;
    
    
    if(hdlc->bitstream & 1) {
        hdlc->bitbuf |= 0x100;
    }
    
    // Got byte
    if (hdlc->bitbuf & 1) {
        
        if (hdlc->rx_ptr >= hdlc->rx_buf+sizeof(hdlc->rx_buf)) {
            hdlc->rx_state = false;
            fprintf(stderr, "Error: packet to large.\n");
            return;
        }
        
        *hdlc->rx_ptr++ = hdlc->bitbuf >> 1;
        hdlc->bitbuf = 0x80;
        
        return;
    }
    
    // Not complete byte, shift right
    hdlc->bitbuf >>= 1;
}
