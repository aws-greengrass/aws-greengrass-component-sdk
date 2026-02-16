#include "gg/ipc/mock.h"
#include "gg/ipc/packet_sequences.h"
#include "packets/packets.h"
#include <assert.h>
#include <gg/buffer.h>
#include <gg/error.h>
#include <gg/map.h>
#include <gg/object.h>
#include <gg/types.h>
#include <gg/vector.h>
#include <time.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>

GgipcPacket gg_test_config_get_object_request_packet(
    int32_t stream_id, GgBufList key_path, GgBuffer *component_name
) {
    static GgObject list[10];
    assert(sizeof(list) / sizeof(list[0]) >= key_path.len);
    for (size_t i = 0; i != key_path.len; ++i) {
        list[i] = gg_obj_buf(key_path.bufs[i]);
    }
    size_t list_len = key_path.len;

    static GgKV pairs[2];
    pairs[0] = gg_kv(
        GG_STR("keyPath"),
        gg_obj_list((GgList) { .items = list, .len = list_len })
    );
    if (component_name != NULL) {
        pairs[1] = gg_kv(GG_STR("componentName"), gg_obj_buf(*component_name));
    }
    size_t pairs_len = sizeof(pairs) / sizeof(pairs[0]);
    if (component_name == NULL) {
        pairs_len--;
    }

    return (GgipcPacket) {
        .direction = CLIENT_TO_SERVER,
        .has_payload = true,
        .payload = gg_obj_map((GgMap) { .pairs = pairs, .len = pairs_len }),
        .headers
        = GG_IPC_REQUEST_HEADERS(stream_id, "aws.greengrass#GetConfiguration"),
        .header_count = GG_IPC_REQUEST_HEADERS_COUNT
    };
}

GgipcPacket gg_test_config_get_object_accepted_packet(
    int32_t stream_id,
    GgBuffer *component_name,
    GgBuffer *last_key,
    GgObject value
) {
    static GgKV payload[2];
    if (last_key == NULL) {
        payload[0] = gg_kv(GG_STR("value"), value);
    } else {
        static GgKV payload_inner[1];
        payload_inner[0] = gg_kv(*last_key, value);
        size_t payload_inner_len
            = sizeof(payload_inner) / sizeof(payload_inner[0]);
        payload[0] = gg_kv(
            GG_STR("value"),
            gg_obj_map((GgMap) { .pairs = payload_inner,
                                 .len = payload_inner_len })
        );
    }
    payload[1] = gg_kv(
        GG_STR("componentName"),
        gg_obj_buf(
            component_name == NULL ? GG_STR("MyComponent") : *component_name
        )
    );
    size_t payload_len = sizeof(payload) / sizeof(payload[0]);

    return (GgipcPacket) {
        .direction = SERVER_TO_CLIENT,
        .has_payload = true,
        .payload = gg_obj_map((GgMap) { .pairs = payload, .len = payload_len }),
        .headers
        = GG_IPC_ACCEPTED_HEADERS(stream_id, "aws.greengrass#GetConfiguration"),
        .header_count = GG_IPC_ACCEPTED_HEADERS_COUNT
    };
}

GgipcPacketSequence gg_test_config_get_object_sequence(
    int32_t stream_id,
    GgBufList key_path,
    GgBuffer *component_name,
    GgObject value
) {
    GgipcPacket request = gg_test_config_get_object_request_packet(
        stream_id, key_path, component_name
    );

    GgBuffer *last_key = NULL;
    if (key_path.len > 0) {
        last_key = &key_path.bufs[key_path.len - 1];
    }

    GgipcPacket response = gg_test_config_get_object_accepted_packet(
        stream_id, component_name, last_key, value
    );
    return (GgipcPacketSequence) { .packets = { request, response }, .len = 2 };
}

GgipcPacketSequence gg_test_config_get_not_found_sequence(int32_t stream_id) {
    GgipcPacket request = gg_test_config_get_object_request_packet(
        stream_id, GG_BUF_LIST(), NULL
    );

    GgipcPacket response = gg_test_ipc_error_packet(
        stream_id,
        GG_STR("ResourceNotFoundError"),
        GG_STR("No such key found in config")
    );

    return (GgipcPacketSequence) { .packets = { request, response }, .len = 2 };
}

// missing payload object
static GgipcPacket config_bad_empty_response(int32_t stream_id) {
    return (GgipcPacket) { .direction = SERVER_TO_CLIENT,
                           .has_payload = true,
                           .payload = gg_obj_map((GgMap) {}),
                           .headers = GG_IPC_ACCEPTED_HEADERS(
                               stream_id, "aws.greengrass#GetConfiguration"
                           ),
                           .header_count = GG_IPC_ACCEPTED_HEADERS_COUNT };
}

GgipcPacketSequence gg_test_config_bad_server_response_sequence(
    int32_t stream_id
) {
    GgipcPacket request = gg_test_config_get_object_request_packet(
        stream_id, GG_BUF_LIST(), NULL
    );

    GgipcPacket response = config_bad_empty_response(stream_id);

    return (GgipcPacketSequence) { .packets = { request, response }, .len = 2 };
}

static GgipcPacket gg_test_config_update_request_packet(
    int32_t stream_id,
    GgList key_path,
    struct timespec timestamp,
    GgObject value
) {
    double timestamp_float
        = (double) timestamp.tv_sec + ((double) (timestamp.tv_nsec) * 1e-9);
    static GgKV pairs[3];
    pairs[0] = gg_kv(GG_STR("keyPath"), gg_obj_list(key_path));
    pairs[1] = gg_kv(GG_STR("timestamp"), gg_obj_f64(timestamp_float));
    pairs[2] = gg_kv(GG_STR("valueToMerge"), value);

    size_t pairs_len = 3;

    return (GgipcPacket) { .has_payload = true,
                           .payload = gg_obj_map((GgMap) { .len = pairs_len,
                                                           .pairs = pairs }),
                           .direction = CLIENT_TO_SERVER,
                           .headers = GG_IPC_REQUEST_HEADERS(
                               stream_id, "aws.greengrass#UpdateConfiguration"
                           ),
                           .header_count = GG_IPC_REQUEST_HEADERS_COUNT };
}

static GgipcPacket gg_test_config_update_response_packet(int32_t stream_id) {
    return (GgipcPacket) { .has_payload = false,
                           .direction = SERVER_TO_CLIENT,
                           .headers = GG_IPC_ACCEPTED_HEADERS(
                               stream_id, "aws.greengrass#UpdateConfiguration"
                           ),
                           .header_count = GG_IPC_ACCEPTED_HEADERS_COUNT };
}

GgipcPacketSequence gg_test_config_update_sequence(
    int32_t stream_id,
    GgBufList key_path,
    struct timespec timestamp,
    GgObject value
) {
    static GgObject objects[GG_MAX_OBJECT_DEPTH - 1U];
    GgObjVec key_path_vec = GG_OBJ_VEC(objects);
    GgError err = GG_ERR_OK;
    GG_BUF_LIST_FOREACH (key, key_path) {
        gg_obj_vec_chain_push(&err, &key_path_vec, gg_obj_buf(*key));
    }
    assert((err == GG_ERR_OK) && ("TEST LOGIC ERROR: key_path too long"));

    GgipcPacket request = gg_test_config_update_request_packet(
        stream_id, key_path_vec.list, timestamp, value
    );
    GgipcPacket response = gg_test_config_update_response_packet(stream_id);
    return (GgipcPacketSequence) { .packets = { request, response }, .len = 2 };
}

/// test with key_path=["key"], timestamp=1 second, value=gg_obj_map(GG_MAP())
GgipcPacketSequence gg_test_config_update_rejected_sequence(int32_t stream_id) {
    static GgObject list[1];
    list[0] = gg_obj_buf(GG_STR("key"));
    GgipcPacket request = gg_test_config_update_request_packet(
        stream_id,
        (GgList) { .items = list, .len = 1 },
        (struct timespec) { .tv_sec = 1, .tv_nsec = 0 },
        gg_obj_map((GgMap) { .len = 0, .pairs = NULL })
    );
    GgipcPacket response = gg_test_ipc_service_error_packet(stream_id);
    return (GgipcPacketSequence) { .packets = { request, response }, .len = 2 };
}

static GgipcPacket gg_test_config_update_packet(
    int32_t stream_id, GgBuffer component_name, GgBufList key_path
) {
    static GgObject list[GG_MAX_OBJECT_DEPTH + 1U];
    GgObjVec vec = GG_OBJ_VEC(list);
    assert(key_path.len < sizeof(list) / sizeof(list[0]));
    GgError ret = GG_ERR_OK;

    GG_BUF_LIST_FOREACH (key, key_path) {
        gg_obj_vec_chain_push(&ret, &vec, gg_obj_buf(*key));
    }
    assert(ret == GG_ERR_OK);

    static GgKV inner_payload[2];
    inner_payload[0] = gg_kv(GG_STR("keyPath"), gg_obj_list(vec.list));
    inner_payload[1]
        = gg_kv(GG_STR("componentName"), gg_obj_buf(component_name));

    size_t inner_payload_len = sizeof(inner_payload) / sizeof(inner_payload[0]);
    static GgKV payload[1];
    payload[0] = gg_kv(
        GG_STR("configurationUpdateEvent"),
        gg_obj_map((GgMap) { .len = inner_payload_len, .pairs = inner_payload })
    );

    size_t payload_len = 1;

    return (GgipcPacket) {
        .direction = SERVER_TO_CLIENT,
        .has_payload = true,
        .payload = gg_obj_map((GgMap) { .pairs = payload, .len = payload_len }),
        .headers = GG_IPC_SUBSCRIBE_MESSAGE_HEADERS(
            stream_id, "aws.greengrass#ConfigurationUpdateEvents"
        ),
        .header_count = GG_IPC_SUBSCRIBE_MESSAGE_HEADERS_COUNT
    };
}

static GgipcPacket gg_test_config_subscribe_request_packet(
    int32_t stream_id, GgBuffer component_name, GgBufList key_path
) {
    static GgObject list[GG_MAX_OBJECT_DEPTH + 1U];
    GgObjVec vec = GG_OBJ_VEC(list);
    assert(key_path.len < sizeof(list) / sizeof(list[0]));
    GgError ret = GG_ERR_OK;

    GG_BUF_LIST_FOREACH (key, key_path) {
        gg_obj_vec_chain_push(&ret, &vec, gg_obj_buf(*key));
    }
    assert(ret == GG_ERR_OK);

    static GgKV payload[2];
    payload[0] = gg_kv(GG_STR("keyPath"), gg_obj_list(vec.list));
    payload[1] = gg_kv(GG_STR("componentName"), gg_obj_buf(component_name));

    size_t payload_len = sizeof(payload) / sizeof(payload[0]);

    return (GgipcPacket) {
        .direction = CLIENT_TO_SERVER,
        .has_payload = true,
        .payload = gg_obj_map((GgMap) { .pairs = payload, .len = payload_len }),
        .headers = GG_IPC_REQUEST_HEADERS(
            stream_id, "aws.greengrass#SubscribeToConfigurationUpdate"
        ),
        .header_count = GG_IPC_SUBSCRIBE_MESSAGE_HEADERS_COUNT
    };
}

static GgipcPacket gg_test_config_subscribe_accepted_packet(int32_t stream_id) {
    return (GgipcPacket) {
        .direction = SERVER_TO_CLIENT,
        .has_payload = false,
        .headers = GG_IPC_SUBSCRIBE_MESSAGE_HEADERS(
            stream_id, "aws.greengrass#SubscribeToConfigurationUpdateRequest"
        ),
        .header_count = GG_IPC_SUBSCRIBE_MESSAGE_HEADERS_COUNT
    };
}

GgipcPacketSequence gg_get_config_subscribe_accepted_sequence(
    int32_t stream_id, GgBuffer component_name, GgBufList key, size_t messages
) {
    GgipcPacket response_packet
        = gg_test_config_update_packet(stream_id, component_name, key);

    GgipcPacketSequence seq
        = { .packets = { gg_test_config_subscribe_request_packet(
                             stream_id, component_name, key
                         ),
                         gg_test_config_subscribe_accepted_packet(stream_id) },
            .len = 2 };

    size_t max_len = (sizeof(seq.packets) / sizeof(seq.packets[0]));
    assert(messages <= (max_len - seq.len));

    for (uint32_t i = 0; (i != messages) && (seq.len != max_len);
         ++i, ++seq.len) {
        seq.packets[seq.len] = response_packet;
    }

    return seq;
}
