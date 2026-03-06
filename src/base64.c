// aws-greengrass-component-sdk - Lightweight AWS IoT Greengrass SDK
// Copyright Amazon.com, Inc. or its affiliates. All Rights Reserved.
// SPDX-License-Identifier: Apache-2.0

#include <gg/arena.h>
#include <gg/base64.h>
#include <gg/buffer.h>
#include <gg/error.h>
#include <string.h>
#include <stdbool.h>
#include <stdint.h>

static bool base64_char_to_byte(char digit, uint8_t *value) {
    if ((digit >= 'A') && (digit <= 'Z')) {
        *value = (uint8_t) (digit - 'A');
    } else if ((digit >= 'a') && (digit <= 'z')) {
        *value = (uint8_t) (digit - 'a' + ('Z' - 'A' + 1));
    } else if ((digit >= '0') && (digit <= '9')) {
        *value = (uint8_t) (digit - '0' + ('Z' - 'A' + 1) + ('z' - 'a' + 1));
    } else if (digit == '+') {
        *value = 62;
    } else if (digit == '/') {
        *value = 63;
    } else {
        return false;
    }
    return true;
}

static bool base64_decode_segment(
    const uint8_t segment[4U], GgBuffer *target, bool *padding
) {
    uint8_t value[3U] = { 0 };
    size_t len = 0U;

    uint8_t decoded = 0U;
    bool ret = base64_char_to_byte((char) segment[0U], &decoded);
    if (!ret) {
        return false;
    }

    value[0U] = (uint8_t) (decoded << 2U);

    ret = base64_char_to_byte((char) segment[1U], &decoded);
    if (!ret) {
        return false;
    }

    value[0U] |= (uint8_t) (decoded >> 4U);
    value[1U] = (uint8_t) (decoded << 4U);

    if (segment[2U] == '=') {
        if (segment[3U] != '=') {
            // non-padding byte after padding
            return false;
        }
        if (value[1U] != 0U) {
            // bad encoding (includes unused bits)
            return false;
        }
        len = 1U;
    } else {
        ret = base64_char_to_byte((char) segment[2U], &decoded);
        if (!ret) {
            return false;
        }

        value[1U] |= (uint8_t) (decoded >> 2U);
        value[2U] = (uint8_t) (decoded << 6U);

        if (segment[3U] == '=') {
            if (value[2U] != 0U) {
                // bad encoding (includes unused bits)
                return false;
            }
            len = 2U;
        } else {
            ret = base64_char_to_byte((char) segment[3U], &decoded);
            if (!ret) {
                return false;
            }

            value[2U] |= decoded;
            len = 3U;
        }
    }

    if (len > target->len) {
        return false;
    }

    memcpy(target->data, value, len);
    *target = gg_buffer_substr(*target, len, SIZE_MAX);
    *padding = len != 3U;

    return true;
}

bool gg_base64_decode(GgBuffer base64, GgBuffer target[static 1]) {
    if ((base64.len % 4) != 0) {
        return false;
    }
    if (target->len < ((base64.len / 4) * 3)) {
        return false;
    }
    GgBuffer out = *target;
    bool last = false;
    for (size_t i = 0; i < base64.len; i += 4) {
        if (last) {
            // Data after padding
            return false;
        }
        bool ret = base64_decode_segment(&base64.data[i], &out, &last);
        if (!ret) {
            return false;
        }
    }
    target->len = (size_t) (out.data - target->data);
    return true;
}

bool gg_base64_decode_in_place(GgBuffer target[static 1]) {
    return gg_base64_decode(*target, target);
}

static const uint8_t BASE64_TABLE[]
    = "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/";

GgError gg_base64_encode(
    GgBuffer buf, GgArena *alloc, GgBuffer result[static 1]
) {
    size_t base64_len = ((buf.len + 2) / 3) * 4;
    uint8_t *mem = GG_ARENA_ALLOCN(alloc, uint8_t, base64_len);
    if (mem == NULL) {
        return GG_ERR_NOMEM;
    }

    size_t chunks = buf.len / 3;
    for (size_t i = 0; i < chunks; i++) {
        uint32_t chunk = (unsigned) buf.data[i * 3] << 16;
        chunk += (unsigned) buf.data[(i * 3) + 1] << 8;
        chunk += (unsigned) buf.data[(i * 3) + 2];

        mem[i * 4] = BASE64_TABLE[chunk >> 18];
        mem[(i * 4) + 1] = BASE64_TABLE[(chunk >> 12) & 0x3F];
        mem[(i * 4) + 2] = BASE64_TABLE[(chunk >> 6) & 0x3F];
        mem[(i * 4) + 3] = BASE64_TABLE[chunk & 0x3F];
    }
    size_t remaining = buf.len % 3;
    if (remaining > 0) {
        uint32_t chunk = (unsigned) buf.data[chunks * 3] << 16;
        if (remaining > 1) {
            chunk += (unsigned) buf.data[(chunks * 3) + 1] << 8;
        }
        mem[chunks * 4] = BASE64_TABLE[chunk >> 18];
        mem[(chunks * 4) + 1] = BASE64_TABLE[(chunk >> 12) & 0x3F];
        if (remaining > 1) {
            mem[(chunks * 4) + 2] = BASE64_TABLE[(chunk >> 6) & 0x3F];
        } else {
            mem[(chunks * 4) + 2] = '=';
        }
        mem[(chunks * 4) + 3] = '=';
    }

    *result = (GgBuffer) { .data = mem, .len = base64_len };
    return GG_ERR_OK;
}

#ifdef GG_SDK_TESTING
#include <gg/test.h>
#include <gg/types.h>
#include <unity.h>
#include <unity_internals.h>

static inline void base64_encode_test(
    GgBuffer encoded, GgBuffer decoded, unsigned line_number
) {
    static uint8_t mem[1024];
    GgArena arena = gg_arena_init(GG_BUF(mem));

    GgBuffer actual;
    gg_test_assert_ok(
        gg_base64_encode(decoded, &arena, &actual),
        "Failed to encode string",
        line_number
    );

    gg_test_assert_buf_equal_str(encoded, actual, NULL, line_number);
}

#define BASE64_ENCODE_TEST(encoded, decoded) \
    base64_encode_test(encoded, decoded, __LINE__)

GG_TEST_DEFINE(base64_encode_okay) {
    BASE64_ENCODE_TEST(GG_STR("emVybyBwYWRkaW5n"), GG_STR("zero padding"));
    BASE64_ENCODE_TEST(GG_STR("aGFzIHBhZGRpbmc="), GG_STR("has padding"));
    BASE64_ENCODE_TEST(GG_STR("dGVzdHN0cmluZw=="), GG_STR("teststring"));
    BASE64_ENCODE_TEST(GG_STR("MTIzNDU2Nzg5MA=="), GG_STR("1234567890"));
    BASE64_ENCODE_TEST(
        GG_STR("VGhlIHF1aWNrIGJyb3duIGZveCBqdW1wcyBvdmVyIHRoZSBsYXp5IGRvZw=="),
        GG_STR("The quick brown fox jumps over the lazy dog")
    );
}

GG_TEST_DEFINE(base64_encode_nomem) {
    uint8_t small_mem[8];
    GgArena small_arena = gg_arena_init(GG_BUF(small_mem));
    GgBuffer unmodified = GG_STR("unmodified");
    GG_TEST_ASSERT_BAD(
        gg_base64_encode(GG_STR("too big"), &small_arena, &unmodified)
    );

    GG_TEST_ASSERT_BUF_EQUAL_STR(GG_STR("unmodified"), unmodified);
}

static inline void base64_decode_test(
    GgBuffer encoded, GgBuffer decoded, unsigned line_number
) {
    static uint8_t target_mem[1024];
    GgBuffer actual = GG_BUF(target_mem);

    UnityAssertEqualNumber(
        true,
        gg_base64_decode(encoded, &actual),
        "Failed to decode value",
        line_number,
        UNITY_DISPLAY_STYLE_INT
    );

    gg_test_assert_buf_equal_str(decoded, actual, NULL, line_number);
}

#define BASE64_DECODE_TEST(encoded, decoded) \
    base64_decode_test(encoded, decoded, __LINE__)

GG_TEST_DEFINE(base64_decode_okay) {
    BASE64_DECODE_TEST(GG_STR("emVybyBwYWRkaW5n"), GG_STR("zero padding"));
    BASE64_DECODE_TEST(GG_STR("aGFzIHBhZGRpbmc="), GG_STR("has padding"));
    BASE64_DECODE_TEST(GG_STR("dGVzdHN0cmluZw=="), GG_STR("teststring"));
    BASE64_DECODE_TEST(GG_STR("MTIzNDU2Nzg5MA=="), GG_STR("1234567890"));
    BASE64_DECODE_TEST(
        GG_STR("VGhlIHF1aWNrIGJyb3duIGZveCBqdW1wcyBvdmVyIHRoZSBsYXp5IGRvZw=="),
        GG_STR("The quick brown fox jumps over the lazy dog")
    );
}

GG_TEST_DEFINE(base64_decode_nomem) {
    uint8_t small_mem[8];
    GgBuffer small_buf = GG_BUF(small_mem);
    TEST_ASSERT_FALSE(gg_base64_decode(GG_STR("dGVzdHN0cmluZw=="), &small_buf));
}

GG_TEST_DEFINE(base64_decode_invalid) {
    static uint8_t mem[1024];
    {
        GgBuffer output = GG_BUF(mem);
        TEST_ASSERT_FALSE(gg_base64_decode(GG_STR("Not a base64"), &output));
    }

    {
        GgBuffer output = GG_BUF(mem);
        // input not a multiple of 4 bytes
        TEST_ASSERT_FALSE(gg_base64_decode(GG_STR("aGFzIHBhZGRpbmc"), &output));
    }

    {
        GgBuffer output = GG_BUF(mem);
        // input contains data after padding
        TEST_ASSERT_FALSE(
            gg_base64_decode(GG_STR("aGFzIHBhZG=Rpbmc"), &output)
        );
    }
}
#endif
