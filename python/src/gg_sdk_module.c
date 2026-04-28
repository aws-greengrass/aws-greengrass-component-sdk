// aws-greengrass-component-sdk - Lightweight AWS IoT Greengrass SDK
// Copyright Amazon.com, Inc. or its affiliates. All Rights Reserved.
// SPDX-License-Identifier: Apache-2.0

#define PY_SSIZE_T_CLEAN
#include <Python.h>
#include <gg/arena.h>
#include <gg/error.h>
#include <gg/ipc/client.h>
#include <gg/ipc/types.h>
#include <gg/map.h>
#include <gg/object.h>
#include <gg/object_visit.h>
#include <gg/sdk.h>
#include <gg/types.h>
#include <gg/vector.h>
#include <math.h>
#include <time.h>
#include <stdbool.h>
#include <stddef.h>
#include <stdio.h>

typedef struct {
    PyObject_HEAD
} SdkObject;

typedef struct {
    PyObject_HEAD
    GgIpcSubscriptionHandle handle;
    PyObject *callback;
} SubscriptionObject;

static PyObject *gg_error_type;

static const char *error_names[] = {
    [GG_ERR_OK] = NULL,
    [GG_ERR_FAILURE] = "FailureError",
    [GG_ERR_RETRY] = "RetryError",
    [GG_ERR_BUSY] = "BusyError",
    [GG_ERR_FATAL] = "FatalError",
    [GG_ERR_INVALID] = "InvalidError",
    [GG_ERR_UNSUPPORTED] = "UnsupportedError",
    [GG_ERR_PARSE] = "ParseError",
    [GG_ERR_RANGE] = "RangeError",
    [GG_ERR_NOMEM] = "NomemError",
    [GG_ERR_NOCONN] = "NoconnError",
    [GG_ERR_NODATA] = "NodataError",
    [GG_ERR_NOENTRY] = "NoentryError",
    [GG_ERR_CONFIG] = "ConfigError",
    [GG_ERR_REMOTE] = "RemoteError",
    [GG_ERR_EXPECTED] = "ExpectedError",
    [GG_ERR_TIMEOUT] = "TimeoutError",
    [GG_ERR_UNAUTHORIZED] = "UnauthorizedError",
    [GG_ERR_CONFLICT] = "ConflictError",
};

#define ERROR_COUNT (sizeof(error_names) / sizeof(error_names[0]))

static PyObject *error_subclasses[ERROR_COUNT];

static PyObject *raise_gg_error(GgError err) {
    PyErr_SetString(error_subclasses[err], gg_strerror(err));
    return NULL;
}

static PyObject *init_component_state_enum(PyObject *module) {
    PyObject *enum_mod = PyImport_ImportModule("enum");
    if (enum_mod == NULL) {
        return NULL;
    }
    PyObject *int_enum = PyObject_GetAttrString(enum_mod, "IntEnum");
    Py_DECREF(enum_mod);
    if (int_enum == NULL) {
        return NULL;
    }

    PyObject *dict = PyDict_New();
    PyObject *running = PyLong_FromLong(GG_COMPONENT_STATE_RUNNING);
    PyObject *errored = PyLong_FromLong(GG_COMPONENT_STATE_ERRORED);
    PyDict_SetItemString(dict, "RUNNING", running);
    PyDict_SetItemString(dict, "ERRORED", errored);
    Py_DECREF(running);
    Py_DECREF(errored);

    PyObject *result
        = PyObject_CallFunction(int_enum, "sO", "ComponentState", dict);
    Py_DECREF(int_enum);
    Py_DECREF(dict);

    if (result != NULL) {
        PyModule_AddObject(module, "ComponentState", result);
    }
    return result;
}

typedef struct {
    PyObject *stack[GG_MAX_OBJECT_DEPTH];
    GgBuffer pending_key[GG_MAX_OBJECT_DEPTH];
    uint8_t depth;
    PyObject *result;
} ConvToPyobjCtx;

static GgError conv_to_pyobj_set_value(ConvToPyobjCtx *ctx, PyObject *val) {
    if (val == NULL) {
        return GG_ERR_NOMEM;
    }
    if (ctx->depth == 0) {
        ctx->result = val;
        return GG_ERR_OK;
    }
    PyObject *parent = ctx->stack[ctx->depth - 1];
    if (PyList_Check(parent)) {
        PyList_Append(parent, val);
    } else {
        PyObject *key = PyUnicode_FromStringAndSize(
            (const char *) ctx->pending_key[ctx->depth - 1].data,
            (Py_ssize_t) ctx->pending_key[ctx->depth - 1].len
        );
        PyDict_SetItem(parent, key, val);
        Py_DECREF(key);
    }
    Py_DECREF(val);
    return GG_ERR_OK;
}

static GgError conv_to_pyobj_on_null(void *ctx) {
    return conv_to_pyobj_set_value(ctx, Py_NewRef(Py_None));
}

static GgError conv_to_pyobj_on_bool(void *ctx, bool val) {
    return conv_to_pyobj_set_value(ctx, PyBool_FromLong(val));
}

static GgError conv_to_pyobj_on_i64(void *ctx, int64_t val) {
    return conv_to_pyobj_set_value(ctx, PyLong_FromLongLong(val));
}

static GgError conv_to_pyobj_on_f64(void *ctx, double val) {
    return conv_to_pyobj_set_value(ctx, PyFloat_FromDouble(val));
}

static GgError conv_to_pyobj_on_buf(void *ctx, GgBuffer val, GgObject *obj) {
    (void) obj;
    return conv_to_pyobj_set_value(
        ctx,
        PyBytes_FromStringAndSize((const char *) val.data, (Py_ssize_t) val.len)
    );
}

static GgError conv_to_pyobj_on_list(void *ctx, GgList val, GgObject *obj) {
    (void) val;
    (void) obj;
    ConvToPyobjCtx *c = ctx;
    PyObject *list = PyList_New(0);
    if (list == NULL) {
        return GG_ERR_NOMEM;
    }
    c->stack[c->depth++] = list;
    return GG_ERR_OK;
}

static GgError conv_to_pyobj_end_list(void *ctx) {
    ConvToPyobjCtx *c = ctx;
    PyObject *list = c->stack[--c->depth];
    return conv_to_pyobj_set_value(c, list);
}

static GgError conv_to_pyobj_on_map(void *ctx, GgMap val, GgObject *obj) {
    (void) val;
    (void) obj;
    ConvToPyobjCtx *c = ctx;
    PyObject *dict = PyDict_New();
    if (dict == NULL) {
        return GG_ERR_NOMEM;
    }
    c->stack[c->depth++] = dict;
    return GG_ERR_OK;
}

static GgError conv_to_pyobj_on_map_key(void *ctx, GgBuffer key, GgKV *kv) {
    (void) kv;
    ConvToPyobjCtx *c = ctx;
    c->pending_key[c->depth - 1] = key;
    return GG_ERR_OK;
}

static GgError conv_to_pyobj_end_map(void *ctx) {
    ConvToPyobjCtx *c = ctx;
    PyObject *dict = c->stack[--c->depth];
    return conv_to_pyobj_set_value(c, dict);
}

static const GgObjectVisitHandlers CONV_TO_PYOBJ_HANDLERS = {
    .on_null = conv_to_pyobj_on_null,
    .on_bool = conv_to_pyobj_on_bool,
    .on_i64 = conv_to_pyobj_on_i64,
    .on_f64 = conv_to_pyobj_on_f64,
    .on_buf = conv_to_pyobj_on_buf,
    .on_list = conv_to_pyobj_on_list,
    .end_list = conv_to_pyobj_end_list,
    .on_map = conv_to_pyobj_on_map,
    .on_map_key = conv_to_pyobj_on_map_key,
    .end_map = conv_to_pyobj_end_map,
};

static PyObject *gg_obj_to_pyobj(GgObject *obj) {
    ConvToPyobjCtx ctx = { .depth = 0, .result = NULL };
    GgError err = gg_obj_visit(&CONV_TO_PYOBJ_HANDLERS, &ctx, obj);
    if (err != GG_ERR_OK) {
        Py_XDECREF(ctx.result);
        PyErr_SetString(PyExc_RuntimeError, "failed to convert GgObject");
        return NULL;
    }
    return ctx.result;
}

typedef enum {
    PY_LEVEL_DEFAULT,
    PY_LEVEL_LIST,
    PY_LEVEL_MAP,
} PyLevelState;

typedef struct {
    PyObject *source[GG_MAX_OBJECT_DEPTH];
    GgObject *dest[GG_MAX_OBJECT_DEPTH];
    void *arrays[GG_MAX_OBJECT_DEPTH];
    Py_ssize_t len[GG_MAX_OBJECT_DEPTH];
    Py_ssize_t pos[GG_MAX_OBJECT_DEPTH];
    uint8_t state[GG_MAX_OBJECT_DEPTH];
    uint8_t index;
} PyConvertLevels;

// NOLINTNEXTLINE(readability-function-cognitive-complexity)
static bool python_to_gg_obj(PyObject *obj, GgArena *arena, GgObject *out) {
    PyConvertLevels state;

    state.index = 0;
    state.source[0] = obj;
    state.dest[0] = out;
    state.state[0] = PY_LEVEL_DEFAULT;
    state.pos[0] = 0;

    uint8_t subobjects = 0;

    while (true) {
        PyObject *cur_src = state.source[state.index];
        uint8_t cur_state = state.state[state.index];
        Py_ssize_t cur_pos = state.pos[state.index];

        switch ((PyLevelState) cur_state) {
        case PY_LEVEL_DEFAULT: {
            if (cur_src == Py_None) {
                *state.dest[state.index] = (GgObject) { 0 };
            } else if (PyBool_Check(cur_src)) {
                *state.dest[state.index] = gg_obj_bool(cur_src == Py_True);
            } else if (PyLong_Check(cur_src)) {
                *state.dest[state.index]
                    = gg_obj_i64(PyLong_AsLongLong(cur_src));
            } else if (PyFloat_Check(cur_src)) {
                *state.dest[state.index]
                    = gg_obj_f64(PyFloat_AsDouble(cur_src));
            } else if (PyBytes_Check(cur_src)) {
                char *data;
                Py_ssize_t len;
                PyBytes_AsStringAndSize(cur_src, &data, &len);
                *state.dest[state.index] = gg_obj_buf((GgBuffer) {
                    .data = (uint8_t *) data, .len = (size_t) len });
            } else if (PyUnicode_Check(cur_src)) {
                Py_ssize_t len;
                const char *data = PyUnicode_AsUTF8AndSize(cur_src, &len);
                *state.dest[state.index] = gg_obj_buf((GgBuffer) {
                    .data = (uint8_t *) data, .len = (size_t) len });
            } else if (PyList_Check(cur_src)) {
                Py_ssize_t len = PyList_Size(cur_src);
                if (len
                    > (Py_ssize_t) (GG_MAX_OBJECT_SUBOBJECTS - subobjects)) {
                    PyErr_SetString(PyExc_OverflowError, "object too large");
                    return false;
                }
                GgObject *items
                    = GG_ARENA_ALLOCN(arena, GgObject, (size_t) len);
                if (items == NULL) {
                    PyErr_SetString(PyExc_OverflowError, "object too large");
                    return false;
                }
                subobjects += (uint8_t) len;
                state.arrays[state.index] = items;
                state.len[state.index] = len;
                state.pos[state.index] = 0;
                state.state[state.index] = PY_LEVEL_LIST;
                continue;
            } else if (PyDict_Check(cur_src)) {
                Py_ssize_t len = PyDict_Size(cur_src);
                if (len * 2
                    > (Py_ssize_t) (GG_MAX_OBJECT_SUBOBJECTS - subobjects)) {
                    PyErr_SetString(PyExc_OverflowError, "object too large");
                    return false;
                }
                GgKV *pairs = GG_ARENA_ALLOCN(arena, GgKV, (size_t) len);
                if (pairs == NULL) {
                    PyErr_SetString(PyExc_OverflowError, "object too large");
                    return false;
                }
                subobjects += (uint8_t) (len * 2);
                state.arrays[state.index] = pairs;
                state.len[state.index] = len;
                state.pos[state.index] = 0;
                state.state[state.index] = PY_LEVEL_MAP;
                continue;
            } else {
                PyErr_SetString(
                    PyExc_TypeError, "unsupported type for GgObject"
                );
                return false;
            }
        } break;
        case PY_LEVEL_LIST: {
            GgObject *items = state.arrays[state.index];
            if (cur_pos == state.len[state.index]) {
                *state.dest[state.index] = gg_obj_list((GgList) {
                    .items = items, .len = (size_t) state.len[state.index] });
                break;
            }

            state.index += 1;
            if (state.index == GG_MAX_OBJECT_DEPTH) {
                PyErr_SetString(PyExc_OverflowError, "object too deep");
                return false;
            }

            state.source[state.index] = PyList_GetItem(cur_src, cur_pos);
            state.dest[state.index] = &items[cur_pos];
            state.state[state.index] = PY_LEVEL_DEFAULT;

            state.pos[state.index - 1] += 1;
            continue;
        }
        case PY_LEVEL_MAP: {
            GgKV *pairs = state.arrays[state.index];
            if (cur_pos == state.len[state.index]) {
                *state.dest[state.index] = gg_obj_map((GgMap) {
                    .pairs = pairs, .len = (size_t) state.len[state.index] });
                break;
            }

            PyObject *key;
            PyObject *value;
            PyDict_Next(cur_src, &state.pos[state.index], &key, &value);
            if (!PyUnicode_Check(key)) {
                PyErr_SetString(PyExc_TypeError, "dict keys must be strings");
                return false;
            }
            Py_ssize_t key_len;
            const char *key_str = PyUnicode_AsUTF8AndSize(key, &key_len);
            GgBuffer key_buf
                = { .data = (uint8_t *) key_str, .len = (size_t) key_len };
            Py_ssize_t idx = state.pos[state.index] - 1;
            pairs[idx] = gg_kv(key_buf, (GgObject) { 0 });

            state.index += 1;
            if (state.index == GG_MAX_OBJECT_DEPTH) {
                PyErr_SetString(PyExc_OverflowError, "object too deep");
                return false;
            }

            state.source[state.index] = value;
            state.dest[state.index] = gg_kv_val(&pairs[idx]);
            state.state[state.index] = PY_LEVEL_DEFAULT;

            continue;
        }
        }

        if (state.index == 0) {
            return true;
        }

        state.index -= 1;
    }
}

static bool pylist_to_buflist(PyObject *list, GgBufVec *vec) {
    Py_ssize_t len = PyList_Size(list);
    for (Py_ssize_t i = 0; i < len; i++) {
        PyObject *item = PyList_GetItem(list, i);
        if (!PyUnicode_Check(item)) {
            PyErr_SetString(PyExc_TypeError, "key path must be list of str");
            return false;
        }
        Py_ssize_t slen;
        const char *s = PyUnicode_AsUTF8AndSize(item, &slen);
        GgBuffer buf = { .data = (uint8_t *) s, .len = (size_t) slen };
        GgError err = gg_buf_vec_push(vec, buf);
        if (err != GG_ERR_OK) {
            PyErr_SetString(PyExc_OverflowError, "key path too long");
            return false;
        }
    }
    return true;
}

static SdkObject *sdk_singleton;

static SubscriptionObject *make_subscription(
    GgIpcSubscriptionHandle handle, PyObject *callback
);

static PyObject *sdk_new(PyTypeObject *type, PyObject *args, PyObject *kwargs) {
    (void) args;
    (void) kwargs;
    if (sdk_singleton != NULL) {
        Py_INCREF(sdk_singleton);
        return (PyObject *) sdk_singleton;
    }
    gg_sdk_init();
    sdk_singleton = (SdkObject *) type->tp_alloc(type, 0);
    return (PyObject *) sdk_singleton;
}

static PyObject *sdk_connect(SdkObject *self, PyObject *args) {
    (void) self;
    (void) args;
    GgError err = ggipc_connect();
    if (err != GG_ERR_OK) {
        return raise_gg_error(err);
    }
    Py_RETURN_NONE;
}

static PyObject *sdk_connect_with_token(
    SdkObject *self, PyObject *args, PyObject *kwargs
) {
    (void) self;
    static char *kwlist[] = { "socket_path", "auth_token", NULL };
    const char *socket_path;
    Py_ssize_t socket_path_len;
    const char *auth_token;
    Py_ssize_t auth_token_len;

    if (!PyArg_ParseTupleAndKeywords(
            args,
            kwargs,
            "s#s#",
            kwlist,
            &socket_path,
            &socket_path_len,
            &auth_token,
            &auth_token_len
        )) {
        return NULL;
    }

    GgBuffer socket_path_buf
        = { .data = (uint8_t *) socket_path, .len = (size_t) socket_path_len };
    GgBuffer auth_token_buf
        = { .data = (uint8_t *) auth_token, .len = (size_t) auth_token_len };

    GgError err = ggipc_connect_with_token(socket_path_buf, auth_token_buf);
    if (err != GG_ERR_OK) {
        return raise_gg_error(err);
    }
    Py_RETURN_NONE;
}

static PyObject *sdk_publish_to_topic(
    SdkObject *self, PyObject *args, PyObject *kwargs
) {
    (void) self;
    static char *kwlist[] = { "topic", "payload", NULL };
    const char *topic;
    Py_ssize_t topic_len;
    PyObject *payload;

    if (!PyArg_ParseTupleAndKeywords(
            args, kwargs, "s#O", kwlist, &topic, &topic_len, &payload
        )) {
        return NULL;
    }

    GgBuffer topic_buf
        = { .data = (uint8_t *) topic, .len = (size_t) topic_len };
    GgError err;

    if (PyBytes_Check(payload)) {
        char *data;
        Py_ssize_t data_len;
        PyBytes_AsStringAndSize(payload, &data, &data_len);
        GgBuffer payload_buf
            = { .data = (uint8_t *) data, .len = (size_t) data_len };
        err = ggipc_publish_to_topic_binary(topic_buf, payload_buf);
    } else if (PyUnicode_Check(payload)) {
        Py_ssize_t data_len;
        const char *data = PyUnicode_AsUTF8AndSize(payload, &data_len);
        GgBuffer payload_buf
            = { .data = (uint8_t *) data, .len = (size_t) data_len };
        err = ggipc_publish_to_topic_binary(topic_buf, payload_buf);
    } else if (PyDict_Check(payload)) {
        uint8_t mem[sizeof(GgObject[GG_MAX_OBJECT_SUBOBJECTS])];
        GgArena arena = gg_arena_init(GG_BUF(mem));
        GgObject obj;
        if (!python_to_gg_obj(payload, &arena, &obj)) {
            return NULL;
        }
        err = ggipc_publish_to_topic_json(topic_buf, gg_obj_into_map(obj));
    } else {
        PyErr_SetString(PyExc_TypeError, "payload must be dict, bytes, or str");
        return NULL;
    }

    if (err != GG_ERR_OK) {
        return raise_gg_error(err);
    }
    Py_RETURN_NONE;
}

static void subscribe_to_topic_trampoline(
    void *ctx, GgBuffer topic, GgObject payload, GgIpcSubscriptionHandle handle
) {
    (void) handle;
    PyGILState_STATE gstate = PyGILState_Ensure();

    PyObject *callback = ctx;
    PyObject *topic_obj = PyUnicode_FromStringAndSize(
        (const char *) topic.data, (Py_ssize_t) topic.len
    );
    if (topic_obj == NULL) {
        PyErr_Print();
        PyGILState_Release(gstate);
        return;
    }
    PyObject *payload_obj = gg_obj_to_pyobj(&payload);
    if (payload_obj == NULL) {
        PyErr_Print();
        Py_DECREF(topic_obj);
        PyGILState_Release(gstate);
        return;
    }

    PyObject *result
        = PyObject_CallFunctionObjArgs(callback, topic_obj, payload_obj, NULL);
    if (result == NULL) {
        PyErr_Print();
    } else {
        Py_DECREF(result);
    }

    Py_DECREF(topic_obj);
    Py_DECREF(payload_obj);
    PyGILState_Release(gstate);
}

static PyObject *sdk_subscribe_to_topic(
    SdkObject *self, PyObject *args, PyObject *kwargs
) {
    (void) self;
    static char *kwlist[] = { "topic", "callback", NULL };
    const char *topic;
    Py_ssize_t topic_len;
    PyObject *callback;

    if (!PyArg_ParseTupleAndKeywords(
            args, kwargs, "s#O", kwlist, &topic, &topic_len, &callback
        )) {
        return NULL;
    }
    if (!PyCallable_Check(callback)) {
        PyErr_SetString(PyExc_TypeError, "callback must be callable");
        return NULL;
    }

    GgBuffer topic_buf
        = { .data = (uint8_t *) topic, .len = (size_t) topic_len };
    GgIpcSubscriptionHandle handle = { 0 };

    GgError err = ggipc_subscribe_to_topic(
        topic_buf, subscribe_to_topic_trampoline, callback, &handle
    );
    if (err != GG_ERR_OK) {
        return raise_gg_error(err);
    }

    return (PyObject *) make_subscription(handle, callback);
}

static PyObject *sdk_publish_to_iot_core(
    SdkObject *self, PyObject *args, PyObject *kwargs
) {
    (void) self;
    static char *kwlist[] = { "topic", "payload", "qos", NULL };
    const char *topic;
    Py_ssize_t topic_len;
    const char *payload;
    Py_ssize_t payload_len;
    int qos;

    if (!PyArg_ParseTupleAndKeywords(
            args,
            kwargs,
            "s#y#i",
            kwlist,
            &topic,
            &topic_len,
            &payload,
            &payload_len,
            &qos
        )) {
        return NULL;
    }

    GgBuffer topic_buf
        = { .data = (uint8_t *) topic, .len = (size_t) topic_len };
    GgBuffer payload_buf
        = { .data = (uint8_t *) payload, .len = (size_t) payload_len };

    GgError err
        = ggipc_publish_to_iot_core(topic_buf, payload_buf, (uint8_t) qos);
    if (err != GG_ERR_OK) {
        return raise_gg_error(err);
    }
    Py_RETURN_NONE;
}

static void subscribe_to_iot_core_trampoline(
    void *ctx, GgBuffer topic, GgBuffer payload, GgIpcSubscriptionHandle handle
) {
    (void) handle;
    PyGILState_STATE gstate = PyGILState_Ensure();

    PyObject *callback = ctx;
    PyObject *topic_obj = PyUnicode_FromStringAndSize(
        (const char *) topic.data, (Py_ssize_t) topic.len
    );
    if (topic_obj == NULL) {
        PyErr_Print();
        PyGILState_Release(gstate);
        return;
    }
    PyObject *payload_obj = PyBytes_FromStringAndSize(
        (const char *) payload.data, (Py_ssize_t) payload.len
    );
    if (payload_obj == NULL) {
        PyErr_Print();
        Py_DECREF(topic_obj);
        PyGILState_Release(gstate);
        return;
    }

    PyObject *result
        = PyObject_CallFunctionObjArgs(callback, topic_obj, payload_obj, NULL);
    if (result == NULL) {
        PyErr_Print();
    } else {
        Py_DECREF(result);
    }

    Py_DECREF(topic_obj);
    Py_DECREF(payload_obj);
    PyGILState_Release(gstate);
}

static PyObject *sdk_subscribe_to_iot_core(
    SdkObject *self, PyObject *args, PyObject *kwargs
) {
    (void) self;
    static char *kwlist[] = { "topic", "qos", "callback", NULL };
    const char *topic;
    Py_ssize_t topic_len;
    int qos;
    PyObject *callback;

    if (!PyArg_ParseTupleAndKeywords(
            args, kwargs, "s#iO", kwlist, &topic, &topic_len, &qos, &callback
        )) {
        return NULL;
    }
    if (!PyCallable_Check(callback)) {
        PyErr_SetString(PyExc_TypeError, "callback must be callable");
        return NULL;
    }

    GgBuffer topic_buf
        = { .data = (uint8_t *) topic, .len = (size_t) topic_len };
    GgIpcSubscriptionHandle handle = { 0 };

    GgError err = ggipc_subscribe_to_iot_core(
        topic_buf,
        (uint8_t) qos,
        subscribe_to_iot_core_trampoline,
        callback,
        &handle
    );
    if (err != GG_ERR_OK) {
        return raise_gg_error(err);
    }

    return (PyObject *) make_subscription(handle, callback);
}

static PyObject *sdk_update_state(
    SdkObject *self, PyObject *args, PyObject *kwargs
) {
    (void) self;
    static char *kwlist[] = { "state", NULL };
    int state;

    if (!PyArg_ParseTupleAndKeywords(args, kwargs, "i", kwlist, &state)) {
        return NULL;
    }

    GgError err = ggipc_update_state((GgComponentState) state);
    if (err != GG_ERR_OK) {
        return raise_gg_error(err);
    }
    Py_RETURN_NONE;
}

static PyObject *sdk_restart_component(
    SdkObject *self, PyObject *args, PyObject *kwargs
) {
    (void) self;
    static char *kwlist[] = { "component_name", NULL };
    const char *name;
    Py_ssize_t name_len;

    if (!PyArg_ParseTupleAndKeywords(
            args, kwargs, "s#", kwlist, &name, &name_len
        )) {
        return NULL;
    }

    GgBuffer name_buf = { .data = (uint8_t *) name, .len = (size_t) name_len };

    GgError err = ggipc_restart_component(name_buf);
    if (err != GG_ERR_OK) {
        return raise_gg_error(err);
    }
    Py_RETURN_NONE;
}

static PyObject *sdk_get_config(
    SdkObject *self, PyObject *args, PyObject *kwargs
) {
    (void) self;
    static char *kwlist[] = { "key_path", "component_name", NULL };
    PyObject *key_path_list;
    const char *component_name = NULL;
    Py_ssize_t component_name_len = 0;

    if (!PyArg_ParseTupleAndKeywords(
            args,
            kwargs,
            "O|z#",
            kwlist,
            &key_path_list,
            &component_name,
            &component_name_len
        )) {
        return NULL;
    }

    GgBuffer kp_bufs[GG_MAX_OBJECT_DEPTH - 1];
    GgBufVec kp_vec = GG_BUF_VEC(kp_bufs);
    if (!pylist_to_buflist(key_path_list, &kp_vec)) {
        return NULL;
    }

    GgBuffer component_name_buf = { .data = (uint8_t *) component_name,
                                    .len = (size_t) component_name_len };
    uint8_t mem[sizeof(GgObject[GG_MAX_OBJECT_SUBOBJECTS])];
    GgBuffer value_mem = { .data = mem, .len = sizeof(mem) };
    GgObject value;

    GgError err = ggipc_get_config(
        kp_vec.buf_list,
        component_name ? &component_name_buf : NULL,
        value_mem,
        &value
    );
    if (err != GG_ERR_OK) {
        return raise_gg_error(err);
    }

    return gg_obj_to_pyobj(&value);
}

static PyObject *sdk_update_config(
    SdkObject *self, PyObject *args, PyObject *kwargs
) {
    (void) self;
    static char *kwlist[] = { "key_path", "value", "timestamp", NULL };
    PyObject *key_path_list;
    PyObject *value_obj;
    PyObject *timestamp_obj = Py_None;

    if (!PyArg_ParseTupleAndKeywords(
            args,
            kwargs,
            "OO|O",
            kwlist,
            &key_path_list,
            &value_obj,
            &timestamp_obj
        )) {
        return NULL;
    }

    GgBuffer kp_bufs[GG_MAX_OBJECT_DEPTH - 1];
    GgBufVec kp_vec = GG_BUF_VEC(kp_bufs);
    if (!pylist_to_buflist(key_path_list, &kp_vec)) {
        return NULL;
    }

    uint8_t mem[sizeof(GgObject[GG_MAX_OBJECT_SUBOBJECTS])];
    GgArena arena = gg_arena_init(GG_BUF(mem));
    GgObject value;
    if (!python_to_gg_obj(value_obj, &arena, &value)) {
        return NULL;
    }

    struct timespec ts;
    struct timespec *ts_ptr = NULL;
    if (timestamp_obj != Py_None) {
        double secs = PyFloat_AsDouble(timestamp_obj);
        if (PyErr_Occurred()) {
            return NULL;
        }
        double integral;
        double frac = modf(secs, &integral);
        ts.tv_sec = (time_t) integral;
        ts.tv_nsec = (long) (frac * 1e9);
        ts_ptr = &ts;
    }

    GgError err = ggipc_update_config(kp_vec.buf_list, ts_ptr, value);
    if (err != GG_ERR_OK) {
        return raise_gg_error(err);
    }
    Py_RETURN_NONE;
}

static void subscribe_to_config_update_trampoline(
    void *ctx,
    GgBuffer component_name,
    GgList key_path,
    GgIpcSubscriptionHandle handle
) {
    (void) handle;
    PyGILState_STATE gstate = PyGILState_Ensure();

    PyObject *callback = ctx;
    PyObject *cn_obj = PyUnicode_FromStringAndSize(
        (const char *) component_name.data, (Py_ssize_t) component_name.len
    );
    if (cn_obj == NULL) {
        PyErr_Print();
        PyGILState_Release(gstate);
        return;
    }
    PyObject *kp_obj = PyList_New((Py_ssize_t) key_path.len);
    if (kp_obj == NULL) {
        PyErr_Print();
        Py_DECREF(cn_obj);
        PyGILState_Release(gstate);
        return;
    }
    for (size_t i = 0; i < key_path.len; i++) {
        GgBuffer buf = gg_obj_into_buf(key_path.items[i]);
        PyList_SET_ITEM(
            kp_obj,
            (Py_ssize_t) i,
            PyUnicode_FromStringAndSize(
                (const char *) buf.data, (Py_ssize_t) buf.len
            )
        );
    }

    PyObject *result
        = PyObject_CallFunctionObjArgs(callback, cn_obj, kp_obj, NULL);
    if (result == NULL) {
        PyErr_Print();
    } else {
        Py_DECREF(result);
    }

    Py_DECREF(cn_obj);
    Py_DECREF(kp_obj);
    PyGILState_Release(gstate);
}

static PyObject *sdk_subscribe_to_configuration_update(
    SdkObject *self, PyObject *args, PyObject *kwargs
) {
    (void) self;
    static char *kwlist[] = { "key_path", "callback", "component_name", NULL };
    PyObject *key_path_list;
    PyObject *callback;
    const char *component_name = NULL;
    Py_ssize_t component_name_len = 0;

    if (!PyArg_ParseTupleAndKeywords(
            args,
            kwargs,
            "OO|z#",
            kwlist,
            &key_path_list,
            &callback,
            &component_name,
            &component_name_len
        )) {
        return NULL;
    }
    if (!PyCallable_Check(callback)) {
        PyErr_SetString(PyExc_TypeError, "callback must be callable");
        return NULL;
    }

    GgBuffer kp_bufs[GG_MAX_OBJECT_DEPTH - 1];
    GgBufVec kp_vec = GG_BUF_VEC(kp_bufs);
    if (!pylist_to_buflist(key_path_list, &kp_vec)) {
        return NULL;
    }

    GgBuffer component_name_buf = { .data = (uint8_t *) component_name,
                                    .len = (size_t) component_name_len };
    GgIpcSubscriptionHandle handle = { 0 };

    GgError err = ggipc_subscribe_to_configuration_update(
        component_name ? &component_name_buf : NULL,
        kp_vec.buf_list,
        subscribe_to_config_update_trampoline,
        callback,
        &handle
    );
    if (err != GG_ERR_OK) {
        return raise_gg_error(err);
    }

    return (PyObject *) make_subscription(handle, callback);
}

static PyObject *sdk_get_thing_shadow(
    SdkObject *self, PyObject *args, PyObject *kwargs
) {
    (void) self;
    static char *kwlist[] = { "thing_name", "shadow_name", NULL };
    const char *thing_name;
    Py_ssize_t thing_name_len;
    const char *shadow_name = NULL;
    Py_ssize_t shadow_name_len = 0;

    if (!PyArg_ParseTupleAndKeywords(
            args,
            kwargs,
            "s#|z#",
            kwlist,
            &thing_name,
            &thing_name_len,
            &shadow_name,
            &shadow_name_len
        )) {
        return NULL;
    }

    GgBuffer thing_name_buf
        = { .data = (uint8_t *) thing_name, .len = (size_t) thing_name_len };
    GgBuffer shadow_name_buf
        = { .data = (uint8_t *) shadow_name, .len = (size_t) shadow_name_len };
    uint8_t buf[4096];
    GgBuffer payload = { .data = buf, .len = sizeof(buf) };

    GgError err = ggipc_get_thing_shadow(
        thing_name_buf, shadow_name ? &shadow_name_buf : NULL, &payload
    );
    if (err != GG_ERR_OK) {
        return raise_gg_error(err);
    }

    return PyBytes_FromStringAndSize(
        (const char *) payload.data, (Py_ssize_t) payload.len
    );
}

static PyObject *sdk_update_thing_shadow(
    SdkObject *self, PyObject *args, PyObject *kwargs
) {
    (void) self;
    static char *kwlist[] = { "thing_name", "payload", "shadow_name", NULL };
    const char *thing_name;
    Py_ssize_t thing_name_len;
    const char *payload;
    Py_ssize_t payload_len;
    const char *shadow_name = NULL;
    Py_ssize_t shadow_name_len = 0;

    if (!PyArg_ParseTupleAndKeywords(
            args,
            kwargs,
            "s#y#|z#",
            kwlist,
            &thing_name,
            &thing_name_len,
            &payload,
            &payload_len,
            &shadow_name,
            &shadow_name_len
        )) {
        return NULL;
    }

    GgBuffer thing_name_buf
        = { .data = (uint8_t *) thing_name, .len = (size_t) thing_name_len };
    GgBuffer shadow_name_buf
        = { .data = (uint8_t *) shadow_name, .len = (size_t) shadow_name_len };
    GgBuffer payload_buf
        = { .data = (uint8_t *) payload, .len = (size_t) payload_len };

    GgError err = ggipc_update_thing_shadow(
        thing_name_buf, shadow_name ? &shadow_name_buf : NULL, payload_buf, NULL
    );
    if (err != GG_ERR_OK) {
        return raise_gg_error(err);
    }
    Py_RETURN_NONE;
}

static PyObject *sdk_delete_thing_shadow(
    SdkObject *self, PyObject *args, PyObject *kwargs
) {
    (void) self;
    static char *kwlist[] = { "thing_name", "shadow_name", NULL };
    const char *thing_name;
    Py_ssize_t thing_name_len;
    const char *shadow_name = NULL;
    Py_ssize_t shadow_name_len = 0;

    if (!PyArg_ParseTupleAndKeywords(
            args,
            kwargs,
            "s#|z#",
            kwlist,
            &thing_name,
            &thing_name_len,
            &shadow_name,
            &shadow_name_len
        )) {
        return NULL;
    }

    GgBuffer thing_name_buf
        = { .data = (uint8_t *) thing_name, .len = (size_t) thing_name_len };
    GgBuffer shadow_name_buf
        = { .data = (uint8_t *) shadow_name, .len = (size_t) shadow_name_len };

    GgError err = ggipc_delete_thing_shadow(
        thing_name_buf, shadow_name ? &shadow_name_buf : NULL
    );
    if (err != GG_ERR_OK) {
        return raise_gg_error(err);
    }
    Py_RETURN_NONE;
}

typedef struct {
    PyObject *list;
} ListShadowsCtx;

static void list_shadows_callback(void *ctx, GgBuffer shadow_name) {
    ListShadowsCtx *c = ctx;
    PyObject *name = PyUnicode_FromStringAndSize(
        (const char *) shadow_name.data, (Py_ssize_t) shadow_name.len
    );
    PyList_Append(c->list, name);
    Py_DECREF(name);
}

static PyObject *sdk_list_named_shadows(
    SdkObject *self, PyObject *args, PyObject *kwargs
) {
    (void) self;
    static char *kwlist[] = { "thing_name", NULL };
    const char *thing_name;
    Py_ssize_t thing_name_len;

    if (!PyArg_ParseTupleAndKeywords(
            args, kwargs, "s#", kwlist, &thing_name, &thing_name_len
        )) {
        return NULL;
    }

    GgBuffer thing_name_buf
        = { .data = (uint8_t *) thing_name, .len = (size_t) thing_name_len };
    ListShadowsCtx ctx = { .list = PyList_New(0) };

    GgError err = ggipc_list_named_shadows_for_thing(
        thing_name_buf, list_shadows_callback, &ctx
    );
    if (err != GG_ERR_OK) {
        Py_DECREF(ctx.list);
        return raise_gg_error(err);
    }

    return ctx.list;
}

static PyObject *subscription_close(SubscriptionObject *self, PyObject *args) {
    (void) args;
    if (self->handle.val != 0) {
        ggipc_close_subscription(self->handle);
        self->handle.val = 0;
    }
    Py_XDECREF(self->callback);
    self->callback = NULL;
    Py_RETURN_NONE;
}

static PyObject *subscription_enter(SubscriptionObject *self, PyObject *args) {
    (void) args;
    Py_INCREF(self);
    return (PyObject *) self;
}

static PyObject *subscription_exit(SubscriptionObject *self, PyObject *args) {
    (void) args;
    return subscription_close(self, NULL);
}

static void subscription_dealloc(SubscriptionObject *self) {
    if (self->handle.val != 0) {
        ggipc_close_subscription(self->handle);
    }
    Py_XDECREF(self->callback);
    Py_TYPE(self)->tp_free((PyObject *) self);
}

static PyMethodDef subscription_methods[] = {
    { "close",
      (PyCFunction) subscription_close,
      METH_NOARGS,
      "Close the subscription." },
    { "__enter__", (PyCFunction) subscription_enter, METH_NOARGS, NULL },
    { "__exit__", (PyCFunction) subscription_exit, METH_VARARGS, NULL },
    { NULL },
};

static PyTypeObject subscription_type = {
    .ob_base = PyVarObject_HEAD_INIT(NULL, 0)
    .tp_name = "gg_sdk.Subscription",
    .tp_basicsize = sizeof(SubscriptionObject),
    .tp_flags = Py_TPFLAGS_DEFAULT,
    .tp_dealloc = (destructor) subscription_dealloc,
    .tp_methods = subscription_methods,
};

static SubscriptionObject *make_subscription(
    GgIpcSubscriptionHandle handle, PyObject *callback
) {
    SubscriptionObject *sub
        = PyObject_New(SubscriptionObject, &subscription_type);
    if (sub != NULL) {
        sub->handle = handle;
        sub->callback = callback;
        Py_INCREF(callback);
    }
    return sub;
}

static PyMethodDef sdk_methods[] = {
    { "connect",
      (PyCFunction) sdk_connect,
      METH_NOARGS,
      "Connect to Greengrass nucleus." },
    { "connect_with_token",
      (PyCFunction) sdk_connect_with_token,
      METH_VARARGS | METH_KEYWORDS,
      "Connect with explicit socket path and auth token." },
    { "publish_to_topic",
      (PyCFunction) sdk_publish_to_topic,
      METH_VARARGS | METH_KEYWORDS,
      "Publish to a local pub/sub topic." },
    { "subscribe_to_topic",
      (PyCFunction) sdk_subscribe_to_topic,
      METH_VARARGS | METH_KEYWORDS,
      "Subscribe to a local pub/sub topic." },
    { "publish_to_iot_core",
      (PyCFunction) sdk_publish_to_iot_core,
      METH_VARARGS | METH_KEYWORDS,
      "Publish an MQTT message to IoT Core." },
    { "subscribe_to_iot_core",
      (PyCFunction) sdk_subscribe_to_iot_core,
      METH_VARARGS | METH_KEYWORDS,
      "Subscribe to MQTT messages from IoT Core." },
    { "update_state",
      (PyCFunction) sdk_update_state,
      METH_VARARGS | METH_KEYWORDS,
      "Update component state." },
    { "restart_component",
      (PyCFunction) sdk_restart_component,
      METH_VARARGS | METH_KEYWORDS,
      "Restart a Greengrass component." },
    { "get_config",
      (PyCFunction) sdk_get_config,
      METH_VARARGS | METH_KEYWORDS,
      "Get component configuration value." },
    { "update_config",
      (PyCFunction) sdk_update_config,
      METH_VARARGS | METH_KEYWORDS,
      "Update component configuration." },
    { "subscribe_to_configuration_update",
      (PyCFunction) sdk_subscribe_to_configuration_update,
      METH_VARARGS | METH_KEYWORDS,
      "Subscribe to configuration updates." },
    { "get_thing_shadow",
      (PyCFunction) sdk_get_thing_shadow,
      METH_VARARGS | METH_KEYWORDS,
      "Get the shadow for a thing." },
    { "update_thing_shadow",
      (PyCFunction) sdk_update_thing_shadow,
      METH_VARARGS | METH_KEYWORDS,
      "Update the shadow for a thing." },
    { "delete_thing_shadow",
      (PyCFunction) sdk_delete_thing_shadow,
      METH_VARARGS | METH_KEYWORDS,
      "Delete the shadow for a thing." },
    { "list_named_shadows",
      (PyCFunction) sdk_list_named_shadows,
      METH_VARARGS | METH_KEYWORDS,
      "List named shadows for a thing." },
    { NULL },
};

static PyTypeObject sdk_type = {
    .ob_base = PyVarObject_HEAD_INIT(NULL, 0)
    .tp_name = "gg_sdk.Sdk",
    .tp_basicsize = sizeof(SdkObject),
    .tp_flags = Py_TPFLAGS_DEFAULT,
    .tp_methods = sdk_methods,
    .tp_new = sdk_new,
};

static PyMethodDef module_methods[] = {
    { NULL },
};

static struct PyModuleDef gg_sdk_module = {
    .m_base = PyModuleDef_HEAD_INIT,
    .m_name = "gg_sdk",
    .m_doc = "Python bindings for the AWS IoT Greengrass Component SDK",
    .m_size = -1,
    .m_methods = module_methods,
};

// NOLINTNEXTLINE(readability-identifier-naming)
PyMODINIT_FUNC PyInit_gg_sdk(void) {
    PyObject *m = PyModule_Create(&gg_sdk_module);
    if (m == NULL) {
        return NULL;
    }

    if (PyType_Ready(&sdk_type) < 0) {
        return NULL;
    }
    Py_INCREF(&sdk_type);
    PyModule_AddObject(m, "Sdk", (PyObject *) &sdk_type);

    if (PyType_Ready(&subscription_type) < 0) {
        return NULL;
    }
    Py_INCREF(&subscription_type);
    PyModule_AddObject(m, "Subscription", (PyObject *) &subscription_type);

    init_component_state_enum(m);

    gg_error_type = PyErr_NewException("gg_sdk.GgError", PyExc_Exception, NULL);
    PyModule_AddObject(m, "GgError", gg_error_type);

    for (int i = 1; i < (int) ERROR_COUNT; i++) {
        char qualname[64];
        snprintf(qualname, sizeof(qualname), "gg_sdk.%s", error_names[i]);
        error_subclasses[i] = PyErr_NewException(qualname, gg_error_type, NULL);
        PyModule_AddObject(m, error_names[i], error_subclasses[i]);
    }

    return m;
}
