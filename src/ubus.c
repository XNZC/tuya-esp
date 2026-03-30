#include "ubus.h"
#include "clog.h"


/* ON/OFF */

enum {
    RC,
    MSG,
    __ON_OFF_MAX,
};

static const struct blobmsg_policy on_off_policy[__ON_OFF_MAX] = {
    [RC] = {.name = "rc", .type = BLOBMSG_TYPE_INT32},
    [MSG] = {.name = "msg", .type = BLOBMSG_TYPE_STRING},
};

static void parse_on_off(struct ubus_request *req, int type, struct blob_attr *msg);
int call_on_off(struct ubus_context* ctx, struct blob_buf* buf, char* port, int pin, int select);


/* GET */

enum{
    G_RC,
    G_MSG,
    G_DATA,
    __GET_MAX,
};

static const struct blobmsg_policy get_policy[__GET_MAX] = {
    [G_RC] = {.name = "rc", .type = BLOBMSG_TYPE_INT32},
    [G_MSG] = {.name = "msg", .type = BLOBMSG_TYPE_STRING},
    [G_DATA] = {.name = "data", .type = BLOBMSG_TYPE_TABLE},
};

static void parse_get(struct ubus_request *req, int type, struct blob_attr *msg);
int call_get(struct ubus_context* ctx, struct blob_buf* buf, char* port, char* sensor, int pin, char* model);


/* DEVICES */

enum {
    D_PORT,
    __DEVICES_MAX,
};

static const struct blobmsg_policy devices_policy[__DEVICES_MAX] = {
    [D_PORT] = {.name = "ports", .type = BLOBMSG_TYPE_ARRAY},
};


static void parse_devices(struct ubus_request *req, int type, struct blob_attr *msg);
int call_devices(struct ubus_context* ctx, struct blob_buf* buf);

/*---*/

int connect_to_ubus(struct ubus_context** ctx) {
    (*ctx) = ubus_connect(NULL);
    if (!(*ctx)) {
        return -1;
    }

    return 0;
}

static void parse_devices(struct ubus_request *req, int type, struct blob_attr *msg) {
    (void) type;
    
    struct blob_buf* res = (struct blob_buf*) req->priv;

    struct blob_attr* ports[__DEVICES_MAX];

    blobmsg_parse(devices_policy, __DEVICES_MAX, ports, blob_data(msg), blob_len(msg));
    if (!ports[D_PORT]) {
        create_log("No port data is available", NULL);
        return;
    }

    struct blob_attr* current;

    size_t len;

    blob_buf_init(res, 0);
    void* token = blobmsg_open_array(res, "devices_arr");

    blobmsg_for_each_attr(current, ports[D_PORT], len) {
        char* _tmp = blobmsg_format_json(current, 1);
        blobmsg_add_string(res, NULL, _tmp);
        free(_tmp);
    }

    blobmsg_close_array(res, token);
}

int call_devices(struct ubus_context* ctx, struct blob_buf* buf) {
    uint32_t id;

    if (ubus_lookup_id(ctx, "esp.service", &id) || ubus_invoke(ctx, id, "devices", NULL, parse_devices, buf, 3000)) {
        create_log("Cannot request data from procd", NULL);
        return -1;
    }

    return 0;
}

static void parse_get(struct ubus_request *req, int type, struct blob_attr *msg) {
    (void) type;
    
    struct blob_buf* res = (struct blob_buf*) req->priv;
    blob_buf_free(res);

    struct blob_attr* get[__GET_MAX];
    
    blobmsg_parse(get_policy, __GET_MAX, get, blob_data(msg), blob_len(msg));

    if (!get[G_RC] || !get[G_MSG]) {
        create_log("get response parsing failed", NULL);
        return;
    }

    blob_buf_init(res, 0);

    void* token;
    if (get[G_DATA]) {
        token = blobmsg_open_table(res, "get_data");
    }else {
        token = blobmsg_open_table(res, "error");
    }

    blobmsg_add_string(res, "msg", blobmsg_get_string(get[G_MSG]));
    blobmsg_add_u32(res, "rc", blobmsg_get_u32(get[G_RC]));
    
    char* _data = NULL;
    if (get[G_DATA]) {
        void* arr_token = blobmsg_open_array(res, "records");

        _data = blobmsg_format_json(get[G_DATA], 1);
        blobmsg_add_string(res, NULL, _data);
        blobmsg_close_array(res, arr_token);
    }

    blobmsg_close_table(res, token);
    free(_data);
}

int call_get(struct ubus_context* ctx, struct blob_buf* buf, char* port, char* sensor, int pin, char* model) {
    uint32_t id;

    if (ubus_lookup_id(ctx, "esp.service", &id)) {
        create_log("Cannot request data from procd", NULL);
        return -1;
    }

    blob_buf_init(buf, 0);
    blobmsg_add_string(buf, "port", port);
    blobmsg_add_string(buf, "sensor", sensor);
    blobmsg_add_u32(buf, "pin", pin);
    blobmsg_add_string(buf, "model", model);

    if (ubus_invoke(ctx, id, "get", buf->buf, parse_get, buf, 3000)){
        create_log("Cannot request data from procd", NULL);
        return -1;
    }

    return 0;
}

static void parse_on_off(struct ubus_request *req, int type, struct blob_attr *msg) {
    (void) type;

    struct blob_buf* res = (struct blob_buf*) req->priv;
    blob_buf_free(res);

    struct blob_attr* on_off_tb[__ON_OFF_MAX];


    blobmsg_parse(on_off_policy, __ON_OFF_MAX, on_off_tb, blob_data(msg), blob_len(msg));
    if (!on_off_tb[RC] || !on_off_tb[MSG]) {
        create_log("On/Off response parsing failed", NULL);
        return;
    }


    blob_buf_init(res, 0);
    void* token = blobmsg_open_table(res, "on_off");
    blobmsg_add_u32(res, "rc", blobmsg_get_u32(on_off_tb[RC]));
    blobmsg_add_string(res, "msg", blobmsg_get_string(on_off_tb[MSG]));
    blobmsg_close_table(res, token);
}

int call_on_off(struct ubus_context* ctx, struct blob_buf* buf, char* port, int pin, int select) {
    uint32_t id;
    char data[4];

    if (select == 0) {
        strncpy(data, "off", 4);
    }else{
        strncpy(data, "on", 4);
    }

    if (ubus_lookup_id(ctx, "esp.service", &id)) {
        create_log("Cannot request data from procd", NULL);
        return -1;
    }

    blob_buf_init(buf, 0);
    blobmsg_add_string(buf, "port", port);
    blobmsg_add_u32(buf, "pin", pin);

    if (ubus_invoke(ctx, id, data, buf->head, parse_on_off, buf, 3000)){
        create_log("Cannot request data from procd", NULL);
        return -1;
    }

    return 0;
}