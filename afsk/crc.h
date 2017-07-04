//
//  crc.h
//  afsk
//
//  Created by Albin Stigö on 09/06/2017.
//  Copyright © 2017 Albin Stigo. All rights reserved.
//

#ifndef crc_h
#define crc_h

#include <stdio.h>

int check_crc_ccitt(const unsigned char *buf, int cnt);

#endif /* crc_h */
