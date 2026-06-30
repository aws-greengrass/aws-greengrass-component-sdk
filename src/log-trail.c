// aws-greengrass-component-sdk - Lightweight AWS IoT Greengrass SDK
// Copyright Amazon.com, Inc. or its affiliates. All Rights Reserved.
// SPDX-License-Identifier: Apache-2.0

#include <gg/log-trail.h>

#ifdef GG_LOG_TRAIL_ENABLED

#include <gg/buffer.h>
#include <gg/error.h>
#include <gg/eventstream/decode.h>
#include <gg/eventstream/types.h>
#include <gg/log.h>
#include <gg/rand.h>
#include <stdarg.h>
#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>
#include <stdio.h>

static uint16_t gen_nonzero_id(void) {
    uint16_t id;
    gg_rand_fill((GgBuffer) { .data = (uint8_t *) &id, .len = sizeof(id) });
    if (id == 0U) {
        id = 1;
    }
    return id;
}

void gg_log_trail_root_begin(const char *kind, const char *fmt, ...) {
    if (gg_log_current_trail_id() != 0U) {
        return;
    }

    uint16_t trace_id = gen_nonzero_id();
    uint16_t span_id = gen_nonzero_id();
    gg_log_set_trail(trace_id, span_id, 0);

    if (fmt != NULL) {
        char buf[128] = { 0 };
        va_list args;
        va_start(args, fmt);
        (void) vsnprintf(buf, sizeof(buf), fmt, args);
        va_end(args);
        GG_LOGI("trace_start: %s %s", kind, buf);
    } else {
        GG_LOGI("trace_start: %s", kind);
    }
}

size_t gg_log_trail_attach_headers(
    EventStreamHeader *headers, size_t headers_capacity
) {
    uint16_t t;
    uint16_t s;
    uint16_t p;
    gg_log_get_trail(&t, &s, &p);
    if (t == 0U) {
        return 0;
    }
    if (headers_capacity < 3U) {
        return 0;
    }

    headers[0] = (EventStreamHeader) {
        GG_STR("T"), { EVENTSTREAM_INT32, { .int32 = (int32_t) t } }
    };
    headers[1] = (EventStreamHeader) {
        GG_STR("S"), { EVENTSTREAM_INT32, { .int32 = (int32_t) s } }
    };
    headers[2] = (EventStreamHeader) {
        GG_STR("P"), { EVENTSTREAM_INT32, { .int32 = (int32_t) p } }
    };
    return 3;
}

bool gg_log_trail_extract_and_apply(EventStreamHeaderIter headers) {
    uint16_t trace_id = 0;
    uint16_t caller_span = 0;
    bool found_t = false;

    EventStreamHeaderIter it = headers;
    EventStreamHeader h;
    while (eventstream_header_next(&it, &h) == GG_ERR_OK) {
        if (gg_buffer_eq(h.name, GG_STR("T"))
            && (h.value.type == EVENTSTREAM_INT32)) {
            trace_id = (uint16_t) h.value.int32;
            found_t = true;
        } else if (gg_buffer_eq(h.name, GG_STR("S"))
                   && (h.value.type == EVENTSTREAM_INT32)) {
            caller_span = (uint16_t) h.value.int32;
        } else {
            // Not T or S; ignore.
        }
    }

    if (!found_t) {
        return false;
    }

    uint16_t fresh_span = gen_nonzero_id();
    gg_log_set_trail(trace_id, fresh_span, caller_span);
    return true;
}

#endif // GG_LOG_TRAIL_ENABLED
