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

#include "modem.h"
#include "hdlc.h"

// Easier to work with stereo files this way
typedef struct frame_t {
    float l;
    float r;
} frame_t;

int main(int argc, const char * argv[]) {
    
    SF_INFO si;
    SNDFILE* sf;
    
    if (argc < 2) {
        fprintf(stderr, "Usage: afsk wavefile\n");
        exit(EXIT_FAILURE);
    }
    
    if((sf = sf_open(argv[1], SFM_READ, &si)) == NULL) {
        fprintf(stderr, "oops: %s\n", sf_strerror(NULL));
        exit(EXIT_FAILURE);
    }
    
    // setup (mo)dem
    afsk_t afsk;
    afsk_init(&afsk, si.samplerate, 1200., 1200, 2200);
    
    int n_frames = 2048;
    frame_t frames[n_frames];
    float samples[n_frames];
    sf_count_t n;

    // read whole wave file
    while((n = sf_readf_float(sf, (float*) frames, n_frames)) > 0) {
        // copy stereo to mono
        for (int i = 0; i < n; i++) {
            samples[i] = frames[i].l;
        }
        
        // process
        afsk_process(&afsk, samples, (uint32_t) n);
    }
    
    printf("decoded: %d\n", afsk.hdlc.n_packets);

    sf_close(sf);
    
    return 0;
}
