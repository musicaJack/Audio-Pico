/*
 * Copyright (c) 2020 Raspberry Pi (Trading) Ltd.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef _PICO_UTIL_BUFFER_H
#define _PICO_UTIL_BUFFER_H

#include "pico/types.h"
#include <stdlib.h>

#ifdef __cplusplus
extern "C" {
#endif

/** \struct mem_buffer
 *  \brief Wrapper structure around a memory buffer
 */
typedef struct mem_buffer {
    size_t size;
    uint8_t *bytes;
    uint8_t flags;
} mem_buffer_t;

// Buffer allocation functions
static inline mem_buffer_t *pico_buffer_alloc(size_t size) {
    mem_buffer_t *buffer = (mem_buffer_t *)malloc(sizeof(mem_buffer_t));
    if (buffer) {
        buffer->size = size;
        buffer->bytes = (uint8_t *)malloc(size);
        buffer->flags = 0;
        if (!buffer->bytes) {
            free(buffer);
            return NULL;
        }
    }
    return buffer;
}

static inline void pico_buffer_free(mem_buffer_t *buffer) {
    if (buffer) {
        if (buffer->bytes) {
            free(buffer->bytes);
        }
        free(buffer);
    }
}

#ifdef __cplusplus
}
#endif

#endif // _PICO_UTIL_BUFFER_H 