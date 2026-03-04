// aws-greengrass-lite - AWS IoT Greengrass runtime for constrained devices
// Copyright Amazon.com, Inc. or its affiliates. All Rights Reserved.
// SPDX-License-Identifier: Apache-2.0

#include <assert.h>
#include <gg/backoff.h>
#include <gg/buffer.h>
#include <gg/error.h>
#include <gg/log.h>
#include <gg/rand.h>
#include <gg/utils.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdlib.h>

static uint64_t backoff_get_rand(void) {
    uint64_t val;
    gg_rand_fill((GgBuffer) { .data = (uint8_t *) &val, .len = sizeof(val) });
    return val;
}

static void backoff_sleep(uint32_t ms) {
    GgError sleep_err = gg_sleep_ms(ms);
    if (sleep_err != GG_ERR_OK) {
        GG_LOGE("Fatal error: unexpected sleep error during backoff.");
        _Exit(1);
    }
}

GgError gg_backoff(
    uint32_t base_ms,
    uint32_t max_ms,
    uint32_t max_attempts,
    GgError (*fn)(void *ctx),
    void *ctx
) {
    if (fn == NULL) {
        assert(false);
        return GG_ERR_UNSUPPORTED;
    }
    if (base_ms == 0) {
        assert(false);
        return GG_ERR_UNSUPPORTED;
    }
    if (max_ms == 0) {
        assert(false);
        return GG_ERR_UNSUPPORTED;
    }

    uint32_t current_max_ms = base_ms;
    uint32_t attempts = 0;
    GgError ret;

    while (true) {
        ret = fn(ctx);
        if (ret == GG_ERR_OK) {
            return GG_ERR_OK;
        }

        if (max_attempts != 0) {
            attempts += 1;
            if (attempts == max_attempts) {
                return ret;
            }
        }

        // Approximately uniform; final wraparound is negligible as rand is
        // 64-bit and ms is 32-bit
        backoff_sleep((uint32_t) (backoff_get_rand() % current_max_ms));

        if (current_max_ms <= (max_ms / 2)) {
            current_max_ms *= 2;
        } else {
            current_max_ms = max_ms;
        }
    }
}

#ifdef GG_SDK_TESTING
#include <gg/test.h>
#include <unity.h>
#include <unity_internals.h>

typedef struct {
    uint32_t attempts;
} BackoffContext;

static void update_context(BackoffContext *context) {
    context->attempts += 1;
}

static GgError backoff_always_fail(void *ctx) {
    update_context(ctx);
    return GG_ERR_FAILURE;
}

static GgError backoff_succeed_after_5_attempts(void *ctx) {
    update_context(ctx);
    if (((BackoffContext *) ctx)->attempts >= 5) {
        return GG_ERR_OK;
    }
    return GG_ERR_FAILURE;
}

static GgError backoff_succeed_after_100_attempts(void *ctx) {
    update_context(ctx);
    if (((BackoffContext *) ctx)->attempts >= 100) {
        return GG_ERR_OK;
    }
    return GG_ERR_FAILURE;
}

GG_TEST_DEFINE(backoff_okay_with_retries) {
    BackoffContext context = { 0 };
    GG_TEST_ASSERT_OK(
        gg_backoff(1, 1, 10, backoff_succeed_after_5_attempts, &context)
    );
    TEST_ASSERT_EQUAL_UINT32(5, context.attempts);
}

GG_TEST_DEFINE(backoff_okay_on_last_attempt) {
    BackoffContext context = { 0 };
    GG_TEST_ASSERT_OK(
        gg_backoff(1, 1, 5, backoff_succeed_after_5_attempts, &context)
    );
    TEST_ASSERT_EQUAL_UINT32(5, context.attempts);
}

GG_TEST_DEFINE(backoff_retry_on_0_max_attempts) {
    BackoffContext context = { 0 };
    GG_TEST_ASSERT_OK(
        gg_backoff(1, 1, 0, backoff_succeed_after_100_attempts, &context)
    );
    TEST_ASSERT_EQUAL_UINT32(100, context.attempts);
}

GG_TEST_DEFINE(backoff_max_attempts) {
    BackoffContext context = { 0 };
    GG_TEST_ASSERT_BAD(gg_backoff(1, 1, 6, backoff_always_fail, &context));
    TEST_ASSERT_EQUAL_UINT32(6, context.attempts);
}

#endif
