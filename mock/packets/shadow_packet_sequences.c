#include "gg/ipc/packet_sequences.h"
#include "packets.h"
#include <gg/ipc/mock.h>
#include <gg/map.h>
#include <gg/object.h>
#include <gg/types.h>
#include <stdbool.h>
#include <stdint.h>

// UpdateThingShadow

GgipcPacket gg_test_shadow_update_request_packet(
    int32_t stream_id,
    GgBuffer thing_name,
    GgBuffer shadow_name,
    GgBuffer payload_base64
) {
    static GgKV pairs[3];
    pairs[0] = gg_kv(GG_STR("thingName"), gg_obj_buf(thing_name));
    pairs[1] = gg_kv(GG_STR("shadowName"), gg_obj_buf(shadow_name));
    pairs[2] = gg_kv(GG_STR("payload"), gg_obj_buf(payload_base64));
    size_t pairs_len = sizeof(pairs) / sizeof(pairs[0]);

    return (GgipcPacket) {
        .direction = CLIENT_TO_SERVER,
        .has_payload = true,
        .payload = gg_obj_map((GgMap) { .pairs = pairs, .len = pairs_len }),
        .headers
        = GG_IPC_REQUEST_HEADERS(stream_id, "aws.greengrass#UpdateThingShadow"),
        .header_count = GG_IPC_REQUEST_HEADERS_COUNT
    };
}

GgipcPacket gg_test_shadow_update_response_packet(
    int32_t stream_id, GgBuffer payload_base64
) {
    static GgKV pairs[1];
    pairs[0] = gg_kv(GG_STR("payload"), gg_obj_buf(payload_base64));
    size_t pairs_len = sizeof(pairs) / sizeof(pairs[0]);

    return (GgipcPacket) { .direction = SERVER_TO_CLIENT,
                           .has_payload = true,
                           .payload = gg_obj_map((GgMap) { .pairs = pairs,
                                                           .len = pairs_len }),
                           .headers = GG_IPC_ACCEPTED_HEADERS(
                               stream_id, "aws.greengrass#UpdateThingShadow"
                           ),
                           .header_count = GG_IPC_ACCEPTED_HEADERS_COUNT };
}

GgipcPacketSequence gg_test_shadow_update_accepted_sequence(
    int32_t stream_id,
    GgBuffer thing_name,
    GgBuffer shadow_name,
    GgBuffer request_payload,
    GgBuffer response_payload
) {
    return (GgipcPacketSequence) {
        .packets = { gg_test_shadow_update_request_packet(
                         stream_id, thing_name, shadow_name, request_payload
                     ),
                     gg_test_shadow_update_response_packet(
                         stream_id, response_payload
                     ) },
        .len = 2
    };
}

GgipcPacketSequence gg_test_shadow_update_error_sequence(
    int32_t stream_id,
    GgBuffer thing_name,
    GgBuffer shadow_name,
    GgBuffer payload_base64
) {
    return (GgipcPacketSequence) {
        .packets = { gg_test_shadow_update_request_packet(
                         stream_id, thing_name, shadow_name, payload_base64
                     ),
                     gg_test_ipc_service_error_packet(stream_id) },
        .len = 2
    };
}

// GetThingShadow

GgipcPacket gg_test_shadow_get_request_packet(
    int32_t stream_id, GgBuffer thing_name, GgBuffer shadow_name
) {
    static GgKV pairs[2];
    pairs[0] = gg_kv(GG_STR("thingName"), gg_obj_buf(thing_name));
    pairs[1] = gg_kv(GG_STR("shadowName"), gg_obj_buf(shadow_name));
    size_t pairs_len = sizeof(pairs) / sizeof(pairs[0]);

    return (GgipcPacket) {
        .direction = CLIENT_TO_SERVER,
        .has_payload = true,
        .payload = gg_obj_map((GgMap) { .pairs = pairs, .len = pairs_len }),
        .headers
        = GG_IPC_REQUEST_HEADERS(stream_id, "aws.greengrass#GetThingShadow"),
        .header_count = GG_IPC_REQUEST_HEADERS_COUNT
    };
}

GgipcPacket gg_test_shadow_get_response_packet(
    int32_t stream_id, GgBuffer payload_base64
) {
    static GgKV pairs[1];
    pairs[0] = gg_kv(GG_STR("payload"), gg_obj_buf(payload_base64));
    size_t pairs_len = sizeof(pairs) / sizeof(pairs[0]);

    return (GgipcPacket) {
        .direction = SERVER_TO_CLIENT,
        .has_payload = true,
        .payload = gg_obj_map((GgMap) { .pairs = pairs, .len = pairs_len }),
        .headers
        = GG_IPC_ACCEPTED_HEADERS(stream_id, "aws.greengrass#GetThingShadow"),
        .header_count = GG_IPC_ACCEPTED_HEADERS_COUNT
    };
}

GgipcPacketSequence gg_test_shadow_get_accepted_sequence(
    int32_t stream_id,
    GgBuffer thing_name,
    GgBuffer shadow_name,
    GgBuffer response_payload
) {
    return (GgipcPacketSequence) {
        .packets
        = { gg_test_shadow_get_request_packet(
                stream_id, thing_name, shadow_name
            ),
            gg_test_shadow_get_response_packet(stream_id, response_payload) },
        .len = 2
    };
}

GgipcPacketSequence gg_test_shadow_get_error_sequence(
    int32_t stream_id, GgBuffer thing_name, GgBuffer shadow_name
) {
    return (GgipcPacketSequence) {
        .packets = { gg_test_shadow_get_request_packet(
                         stream_id, thing_name, shadow_name
                     ),
                     gg_test_ipc_service_error_packet(stream_id) },
        .len = 2
    };
}

// DeleteThingShadow

GgipcPacket gg_test_shadow_delete_request_packet(
    int32_t stream_id, GgBuffer thing_name, GgBuffer shadow_name
) {
    static GgKV pairs[2];
    pairs[0] = gg_kv(GG_STR("thingName"), gg_obj_buf(thing_name));
    pairs[1] = gg_kv(GG_STR("shadowName"), gg_obj_buf(shadow_name));
    size_t pairs_len = sizeof(pairs) / sizeof(pairs[0]);

    return (GgipcPacket) {
        .direction = CLIENT_TO_SERVER,
        .has_payload = true,
        .payload = gg_obj_map((GgMap) { .pairs = pairs, .len = pairs_len }),
        .headers
        = GG_IPC_REQUEST_HEADERS(stream_id, "aws.greengrass#DeleteThingShadow"),
        .header_count = GG_IPC_REQUEST_HEADERS_COUNT
    };
}

GgipcPacket gg_test_shadow_delete_response_packet(
    int32_t stream_id, GgBuffer payload_base64
) {
    static GgKV pairs[1];
    pairs[0] = gg_kv(GG_STR("payload"), gg_obj_buf(payload_base64));
    size_t pairs_len = sizeof(pairs) / sizeof(pairs[0]);

    return (GgipcPacket) { .direction = SERVER_TO_CLIENT,
                           .has_payload = true,
                           .payload = gg_obj_map((GgMap) { .pairs = pairs,
                                                           .len = pairs_len }),
                           .headers = GG_IPC_ACCEPTED_HEADERS(
                               stream_id, "aws.greengrass#DeleteThingShadow"
                           ),
                           .header_count = GG_IPC_ACCEPTED_HEADERS_COUNT };
}

GgipcPacketSequence gg_test_shadow_delete_accepted_sequence(
    int32_t stream_id,
    GgBuffer thing_name,
    GgBuffer shadow_name,
    GgBuffer response_payload
) {
    return (GgipcPacketSequence) { .packets
                                   = { gg_test_shadow_delete_request_packet(
                                           stream_id, thing_name, shadow_name
                                       ),
                                       gg_test_shadow_delete_response_packet(
                                           stream_id, response_payload
                                       ) },
                                   .len = 2 };
}

GgipcPacketSequence gg_test_shadow_delete_error_sequence(
    int32_t stream_id, GgBuffer thing_name, GgBuffer shadow_name
) {
    return (GgipcPacketSequence) {
        .packets = { gg_test_shadow_delete_request_packet(
                         stream_id, thing_name, shadow_name
                     ),
                     gg_test_ipc_service_error_packet(stream_id) },
        .len = 2
    };
}

// ListNamedShadowsForThing

GgipcPacket gg_test_shadow_list_request_packet(
    int32_t stream_id, GgBuffer thing_name, GgBuffer *next_token
) {
    static GgKV pairs[2];
    pairs[0] = gg_kv(GG_STR("thingName"), gg_obj_buf(thing_name));
    size_t pairs_len = 1;
    if (next_token != NULL) {
        pairs[pairs_len++]
            = gg_kv(GG_STR("nextToken"), gg_obj_buf(*next_token));
    }

    return (GgipcPacket) {
        .direction = CLIENT_TO_SERVER,
        .has_payload = true,
        .payload = gg_obj_map((GgMap) { .pairs = pairs, .len = pairs_len }),
        .headers = GG_IPC_REQUEST_HEADERS(
            stream_id, "aws.greengrass#ListNamedShadowsForThing"
        ),
        .header_count = GG_IPC_REQUEST_HEADERS_COUNT
    };
}

GgipcPacket gg_test_shadow_list_response_packet(
    int32_t stream_id, GgList results, double timestamp, GgBuffer *next_token
) {
    static GgKV pairs[3];
    pairs[0] = gg_kv(GG_STR("results"), gg_obj_list(results));
    pairs[1] = gg_kv(GG_STR("timestamp"), gg_obj_f64(timestamp));
    size_t pairs_len = 2;
    if (next_token != NULL) {
        pairs[pairs_len++]
            = gg_kv(GG_STR("nextToken"), gg_obj_buf(*next_token));
    }

    return (GgipcPacket) {
        .direction = SERVER_TO_CLIENT,
        .has_payload = true,
        .payload = gg_obj_map((GgMap) { .pairs = pairs, .len = pairs_len }),
        .headers = GG_IPC_ACCEPTED_HEADERS(
            stream_id, "aws.greengrass#ListNamedShadowsForThing"
        ),
        .header_count = GG_IPC_ACCEPTED_HEADERS_COUNT
    };
}

GgipcPacketSequence gg_test_shadow_list_accepted_sequence(
    int32_t stream_id,
    GgBuffer thing_name,
    GgBuffer *request_next_token,
    GgList results,
    double timestamp,
    GgBuffer *response_next_token
) {
    return (GgipcPacketSequence) {
        .packets = { gg_test_shadow_list_request_packet(
                         stream_id, thing_name, request_next_token
                     ),
                     gg_test_shadow_list_response_packet(
                         stream_id, results, timestamp, response_next_token
                     ) },
        .len = 2
    };
}

GgipcPacketSequence gg_test_shadow_list_error_sequence(
    int32_t stream_id, GgBuffer thing_name
) {
    return (GgipcPacketSequence) {
        .packets
        = { gg_test_shadow_list_request_packet(stream_id, thing_name, NULL),
            gg_test_ipc_service_error_packet(stream_id) },
        .len = 2
    };
}
