// aws-greengrass-component-sdk - Lightweight AWS IoT Greengrass SDK
// Copyright Amazon.com, Inc. or its affiliates. All Rights Reserved.
// SPDX-License-Identifier: Apache-2.0

#ifndef GG_IPC_CLIENT_B64_ENCODE_MEM_H
#define GG_IPC_CLIENT_B64_ENCODE_MEM_H

#include <gg/attr.h>
#include <gg/ipc/limits.h>
#include <inttypes.h>
#include <pthread.h>

VISIBILITY(hidden)
extern uint8_t gg_ipc_b64_encode_mem[GG_IPC_MAX_MSG_LEN];
VISIBILITY(hidden)
extern pthread_mutex_t gg_ipc_b64_encode_mtx;

#endif
