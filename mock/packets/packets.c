
#include "packets.h"
#include "gg/eventstream/rpc.h"
#include <gg/ipc/mock.h>
#include <gg/map.h>
#include <stdbool.h>
#include <stdlib.h>

GgipcPacket gg_test_ipc_error_packet(
    int32_t stream_id, GgBuffer error_code, GgBuffer message
) {
    static GgKV pairs[2];
    pairs[0] = gg_kv(GG_STR("_errorCode"), gg_obj_buf(error_code));
    pairs[1] = gg_kv(GG_STR("_message"), gg_obj_buf(message));

    size_t pairs_len = sizeof(pairs) / sizeof(pairs[0]);

    return (GgipcPacket) {
        .direction = SERVER_TO_CLIENT,
        .has_payload = true,
        .payload = gg_obj_map((GgMap) { .pairs = pairs, .len = pairs_len }),
        .headers
        = { { GG_STR(":message-type"),
              { EVENTSTREAM_INT32, .int32 = EVENTSTREAM_APPLICATION_ERROR } },
            { GG_STR(":message-flags"),
              { EVENTSTREAM_INT32, .int32 = EVENTSTREAM_TERMINATE_STREAM } },
            { GG_STR(":stream-id"), { EVENTSTREAM_INT32, .int32 = stream_id } },
            { GG_STR(":content-type"),
              { EVENTSTREAM_STRING, .string = GG_STR("application/json") } },
            { GG_STR("service-model-type"),
              { EVENTSTREAM_STRING,
                .string = GG_STR("aws.greengrass#ServiceError") } } },
        .header_count = GG_IPC_REQUEST_HEADERS_COUNT
    };
}

GgipcPacket gg_test_ipc_service_error_packet(int32_t stream_id) {
    return gg_test_ipc_error_packet(
        stream_id, GG_STR("ServiceError"), GG_STR("Unknown service model.")
    );
}

GgipcPacket gg_test_ipc_permissions_error_packet(int32_t stream_id) {
    return gg_test_ipc_error_packet(
        stream_id,
        GG_STR("UnauthorizedError"),
        GG_STR("Component is not permitted to use this IPC method.")
    );
}
