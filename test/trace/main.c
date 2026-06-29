// aws-greengrass-component-sdk - Lightweight AWS IoT Greengrass SDK
// Copyright Amazon.com, Inc. or its affiliates. All Rights Reserved.
// SPDX-License-Identifier: Apache-2.0

#include <assert.h>
#include <gg/buffer.h>
#include <gg/error.h>
#include <gg/eventstream/decode.h>
#include <gg/eventstream/encode.h>
#include <gg/eventstream/types.h>
#include <gg/io.h>
#include <gg/log.h>
#include <gg/trace.h>
#include <pthread.h>
#include <string.h>
#include <unistd.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>

// === TLS / format tests (existing) ===

// Test a) initial state and set
static void test_initial_and_set(void) {
    assert(gg_log_current_trace_id() == 0);
    gg_log_set_trace(0xA34F, 0x34BD, 0);
    assert(gg_log_current_trace_id() != 0);
    assert(gg_log_current_trace_id() == 0xA34F);
    gg_log_clear_trace();
    printf("  PASS: test_initial_and_set\n");
}

// Test b) get returns exact values; clear resets
static void test_get_and_clear(void) {
    gg_log_set_trace(0x1234, 0x5678, 0x9ABC);
    uint16_t t, s, p;
    gg_log_get_trace(&t, &s, &p);
    assert(t == 0x1234);
    assert(s == 0x5678);
    assert(p == 0x9ABC);
    gg_log_clear_trace();
    assert(gg_log_current_trace_id() == 0);
    gg_log_get_trace(&t, &s, &p);
    assert(t == 0 && s == 0 && p == 0);
    printf("  PASS: test_get_and_clear\n");
}

// Test c) multi-thread isolation
struct thread_args {
    uint16_t trace_id;
    uint16_t span_id;
    uint16_t parent_span_id;
    int ok;
};

static void *thread_fn(void *arg) {
    struct thread_args *a = (struct thread_args *) arg;
    gg_log_set_trace(a->trace_id, a->span_id, a->parent_span_id);
    // Sleep briefly to allow interleaving
    usleep(50000);
    uint16_t t, s, p;
    gg_log_get_trace(&t, &s, &p);
    a->ok = (t == a->trace_id && s == a->span_id && p == a->parent_span_id);
    return NULL;
}

static void test_thread_isolation(void) {
    struct thread_args a1 = {
        .trace_id = 0xAAAA, .span_id = 0xBBBB, .parent_span_id = 0xCCCC, .ok = 0
    };
    struct thread_args a2 = {
        .trace_id = 0x1111, .span_id = 0x2222, .parent_span_id = 0x3333, .ok = 0
    };
    pthread_t t1, t2;
    int rc1 = pthread_create(&t1, NULL, thread_fn, &a1);
    int rc2 = pthread_create(&t2, NULL, thread_fn, &a2);
    assert(rc1 == 0);
    assert(rc2 == 0);
    pthread_join(t1, NULL);
    pthread_join(t2, NULL);
    assert(a1.ok);
    assert(a2.ok);
    printf("  PASS: test_thread_isolation\n");
}

// Test d) log prefix format
static void test_log_prefix_format(void) {
    char tmppath[] = "/tmp/gg_trace_test_XXXXXX";
    int fd = mkstemp(tmppath);
    assert(fd >= 0);

    // Redirect stderr to temp file
    FILE *old_stderr = stderr;
    FILE *tmp = fdopen(fd, "w+");
    assert(tmp != NULL);
    stderr = tmp;

    // Log with trace active (parent=0 -> "----")
    gg_log_set_trace(0xA34F, 0x34BD, 0);
    GG_LOGI("hello traced");
    fflush(stderr);

    // Read back
    fseek(tmp, 0, SEEK_SET);
    char buf[512] = { 0 };
    size_t n = fread(buf, 1, sizeof(buf) - 1, tmp);
    (void) n;

    // Assert trace bracket present with ---- parent
    assert(strstr(buf, "[A34F:34BD:----:") != NULL);
    assert(strstr(buf, "hello traced") != NULL);

    // Now clear trace and log again
    gg_log_clear_trace();
    long pos = ftell(tmp);
    GG_LOGI("hello untraced");
    fflush(stderr);

    // Read the new line
    fseek(tmp, pos, SEEK_SET);
    char buf2[512] = { 0 };
    n = fread(buf2, 1, sizeof(buf2) - 1, tmp);
    (void) n;

    // Untraced line should have no bracket, should have file:line: format
    assert(strstr(buf2, "[A34F:") == NULL);
    assert(strstr(buf2, "hello untraced") != NULL);

    // Restore stderr
    stderr = old_stderr;
    fclose(tmp);
    unlink(tmppath);
    printf("  PASS: test_log_prefix_format\n");
}

// === Orchestration tests (ported from ggl-trace) ===

static void test_root_begin_generates_nonzero_and_idempotent(void) {
    gg_log_clear_trace();

    gg_trace_root_begin("test_op", "key=%s", "value");

    uint16_t t, s, p;
    gg_log_get_trace(&t, &s, &p);
    assert(t != 0);
    assert(s != 0);
    assert(p == 0);
    assert(gg_log_current_trace_id() == t);

    // Idempotent: second call does not change IDs
    uint16_t t2, s2, p2;
    gg_trace_root_begin("other_op", NULL);
    gg_log_get_trace(&t2, &s2, &p2);
    assert(t2 == t);
    assert(s2 == s);
    assert(p2 == p);

    gg_log_clear_trace();
    printf("  PASS: root_begin generates non-zero IDs and is idempotent\n");
}

static void test_attach_headers(void) {
    EventStreamHeader hdrs[3];

    // No trace active -> returns 0
    gg_log_clear_trace();
    size_t n_inactive = gg_trace_attach_headers(hdrs, 3);
    assert(n_inactive == 0);

    // Set a known trace
    gg_log_set_trace(0xBEEF, 0xCAFE, 0x1234);
    size_t n_active = gg_trace_attach_headers(hdrs, 3);
    assert(n_active == 3);

    assert(gg_buffer_eq(hdrs[0].name, GG_STR("T")));
    assert(hdrs[0].value.int32 == (int32_t) 0xBEEF);
    assert(gg_buffer_eq(hdrs[1].name, GG_STR("S")));
    assert(hdrs[1].value.int32 == (int32_t) 0xCAFE);
    assert(gg_buffer_eq(hdrs[2].name, GG_STR("P")));
    assert(hdrs[2].value.int32 == (int32_t) 0x1234);

    // Capacity < 3 -> returns 0
    size_t n_small = gg_trace_attach_headers(hdrs, 2);
    assert(n_small == 0);

    gg_log_clear_trace();
    printf("  PASS: attach_headers correct with/without active trace\n");
}

/// Encode headers into wire format and decode back to get an iterator.
/// Returns the decoded EventStreamMessage (caller must keep wire_buf alive).
static EventStreamMessage encode_decode_headers(
    EventStreamHeader *headers, size_t count, uint8_t *wire_buf, size_t buf_len
) {
    GgBuffer buf = { .data = wire_buf, .len = buf_len };
    GgError err = eventstream_encode(&buf, headers, count, GG_NULL_READER);
    assert(err == GG_ERR_OK);

    // Decode: first 12 bytes are the prelude
    GgBuffer prelude_buf = { .data = wire_buf, .len = 12 };
    EventStreamPrelude prelude;
    err = eventstream_decode_prelude(prelude_buf, &prelude);
    assert(err == GG_ERR_OK);

    GgBuffer data_section = { .data = wire_buf + 12, .len = prelude.data_len };
    EventStreamMessage msg;
    err = eventstream_decode(&prelude, data_section, &msg);
    assert(err == GG_ERR_OK);

    return msg;
}

static void test_round_trip(void) {
    gg_log_clear_trace();
    gg_log_set_trace(0xA34F, 0x34BD, 0);

    EventStreamHeader hdrs[3];
    size_t n = gg_trace_attach_headers(hdrs, 3);
    assert(n == 3);

    // Encode to wire and decode back to get an EventStreamHeaderIter
    uint8_t wire_buf[256];
    EventStreamMessage msg = encode_decode_headers(hdrs, 3, wire_buf, 256);

    gg_log_clear_trace();

    bool applied = gg_trace_extract_and_apply(msg.headers);
    assert(applied);

    uint16_t t, s, p;
    gg_log_get_trace(&t, &s, &p);
    assert(t == 0xA34F);
    assert(s != 0);
    assert(s != 0x34BD); // fresh span, not caller's
    assert(p == 0x34BD); // parent = caller's span

    gg_log_clear_trace();
    printf(
        "  PASS: round-trip preserves trace_id, fresh span, parent=caller\n"
    );
}

static void test_extract_no_t_header(void) {
    // Set a sentinel trace
    gg_log_set_trace(0x1111, 0x2222, 0x3333);

    // Headers without T — just method and type (like a normal core-bus frame)
    EventStreamHeader hdrs[2] = {
        { GG_STR("method"), { EVENTSTREAM_STRING, .string = GG_STR("test") } },
        { GG_STR("type"), { EVENTSTREAM_INT32, { .int32 = 1 } } },
    };

    uint8_t wire_buf[256];
    EventStreamMessage msg = encode_decode_headers(hdrs, 2, wire_buf, 256);

    bool applied = gg_trace_extract_and_apply(msg.headers);
    assert(!applied);

    // TLS unchanged
    uint16_t t, s, p;
    gg_log_get_trace(&t, &s, &p);
    assert(t == 0x1111);
    assert(s == 0x2222);
    assert(p == 0x3333);

    gg_log_clear_trace();
    printf("  PASS: extract_and_apply returns false without T header\n");
}

static void test_extract_wrong_type_t_header(void) {
    // Set a sentinel trace
    gg_log_set_trace(0x1111, 0x2222, 0x3333);

    // T header with wrong type (STRING instead of INT32)
    EventStreamHeader hdrs[2] = {
        { GG_STR("T"), { EVENTSTREAM_STRING, .string = GG_STR("garbage") } },
        { GG_STR("S"), { EVENTSTREAM_INT32, { .int32 = 0x00FF } } },
    };

    uint8_t wire_buf[256];
    EventStreamMessage msg = encode_decode_headers(hdrs, 2, wire_buf, 256);

    bool applied = gg_trace_extract_and_apply(msg.headers);
    assert(!applied);

    // TLS unchanged (sentinel preserved)
    uint16_t t, s, p;
    gg_log_get_trace(&t, &s, &p);
    assert(t == 0x1111);
    assert(s == 0x2222);
    assert(p == 0x3333);

    gg_log_clear_trace();
    printf(
        "  PASS: extract_and_apply returns false with wrong-type T header\n"
    );
}

int main(void) {
    printf("trace_test:\n");
    test_initial_and_set();
    test_get_and_clear();
    test_thread_isolation();
    test_log_prefix_format();
    test_root_begin_generates_nonzero_and_idempotent();
    test_attach_headers();
    test_round_trip();
    test_extract_no_t_header();
    test_extract_wrong_type_t_header();
    printf("ALL PASS\n");
    return 0;
}
