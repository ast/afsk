//
//  ringbuffer_h
//  afsk
//
//  Created by Albin Stigö on 09/06/2017.
//  Copyright © 2017 Albin Stigo. All rights reserved.
//

// Inspired by:
// https://www.snellman.net/blog/archive/2016-12-13-ring-buffers/

#ifndef ringbuffer_h
#define ringbuffer_h

#include <stdbool.h>
#include <stdint.h>
#include <assert.h>

static const int MAX_CAPACITY = 512;

typedef struct ringbuffer {
    // Just statically allocate a max_capacity buffer.
    // We are not memory constrained anyway.
    float buffer[MAX_CAPACITY];
    uint32_t capacity;
    uint32_t read;
    uint32_t write;
} ringbuffer_t;

// Mask indicies to max capacity.
static inline uint32_t rb_mask(uint32_t val) {
    return val & (MAX_CAPACITY - 1);
}

static inline bool rb_empty(ringbuffer_t* rb) { return rb->read == rb->write; }
static inline uint32_t rb_size(ringbuffer_t* rb) { return rb->write - rb->read; }
static inline bool rb_full(ringbuffer_t* rb) { return rb_size(rb) == rb->capacity; }

static inline void rb_push(ringbuffer_t *rb, float val) {
    assert(!rb_full(rb));
    rb->buffer[rb_mask(rb->write++)] = val;
}

static inline float rb_read(ringbuffer_t* rb, uint32_t i) {
    assert(rb_size(rb) > i);
    return rb->buffer[rb_mask(rb->read + i)];
}

static inline float rb_shift(ringbuffer_t* rb) {
    assert(!rb_empty(rb));
    return rb->buffer[rb_mask(rb->read++)];
}

void        rb_init(ringbuffer_t* rb, uint32_t capacity);
void        rb_fill(ringbuffer_t* rb, float val);

#endif /* ringbuffer_h */
