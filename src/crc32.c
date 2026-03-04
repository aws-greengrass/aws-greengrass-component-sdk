// aws-greengrass-component-sdk - Lightweight AWS IoT Greengrass SDK
// Copyright Amazon.com, Inc. or its affiliates. All Rights Reserved.
// SPDX-License-Identifier: Apache-2.0

#include "crc32.h"
#include <gg/buffer.h>
#include <stddef.h>
#include <stdint.h>

#ifdef __has_builtin
#if __has_builtin(__builtin_rev_crc32_data8)
#define HAS_BUILTIN_CRC 1
#endif
#endif

#ifndef HAS_BUILTIN_CRC
#define HAS_BUILTIN_CRC 0
#endif

#if HAS_BUILTIN_CRC

static uint32_t crc_step(uint32_t crc, uint8_t byte) {
    return __builtin_rev_crc32_data8(crc, byte, 0x04C11DB7L);
}

#else

// CRC code adapted from rfc1952 GZIP file format specification version 4.3

/// Table of CRCs of all 8-bit messages.
/// Initialized by `make_crc_table`.
static uint32_t crc_table[256];

/// Make the table for a fast CRC.
__attribute__((constructor)) static void make_crc_table(void) {
    for (uint32_t n = 0; n < 256; n++) {
        uint32_t c = n;
        for (int k = 0; k < 8; k++) {
            if (c & 1) {
                c = 0xEDB88320L ^ (c >> 1);
            } else {
                c = c >> 1;
            }
        }
        crc_table[n] = c;
    }
}

static uint32_t crc_step(uint32_t crc, uint8_t byte) {
    return crc_table[(crc ^ byte) & 0xFF] ^ (crc >> 8);
}

#endif

uint32_t gg_update_crc(uint32_t crc, GgBuffer buf) {
    uint32_t c = ~crc;
    for (size_t n = 0; n < buf.len; n++) {
        c = crc_step(c, buf.data[n]);
    }
    return ~c;
}

#ifdef GG_SDK_TESTING

#include <gg/test.h>
#include <unity.h>

GG_TEST_DEFINE(crc_check) {
    // CRC of empty string does not update the CRC
    TEST_ASSERT_EQUAL_UINT32(0, gg_update_crc(0, GG_STR("")));
    TEST_ASSERT_EQUAL_UINT32(1234, gg_update_crc(1234, GG_STR("")));

    /* The first four of the last eight bytes of gzip's compressed output
    is the uncompressed input's CRC. Test values created with this command:

    echo -n "teststring" | gzip -1 | tail -c 8 | head -c 4 | \
    hexdump -e '1/4 "0x%08XU" "\n"'

    */

    TEST_ASSERT_EQUAL_UINT32(
        0x1F58F83EU, gg_update_crc(0, GG_STR("teststring"))
    );
    TEST_ASSERT_EQUAL_UINT32(
        0xCBF43926U, gg_update_crc(0, GG_STR("123456789"))
    );
    TEST_ASSERT_EQUAL_UINT32(
        0x414FA339U,
        gg_update_crc(0, GG_STR("The quick brown fox jumps over the lazy dog"))
    );
}

#endif
