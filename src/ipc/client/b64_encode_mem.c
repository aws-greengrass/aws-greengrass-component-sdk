// aws-greengrass-component-sdk - Lightweight AWS IoT Greengrass SDK
// Copyright Amazon.com, Inc. or its affiliates. All Rights Reserved.
// SPDX-License-Identifier: Apache-2.0

#include "b64_encode_mem.h"
#include <gg/arena.h>
#include <gg/base64.h>
#include <gg/buffer.h>
#include <gg/cleanup.h>
#include <gg/error.h>
#include <gg/ipc/client.h>
#include <gg/ipc/limits.h>
#include <gg/log.h>
#include <inttypes.h>
#include <pthread.h>

uint8_t gg_ipc_b64_encode_mem[GG_IPC_MAX_MSG_LEN] = { 0 };
pthread_mutex_t gg_ipc_b64_encode_mtx = PTHREAD_MUTEX_INITIALIZER;
