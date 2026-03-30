#include "tuya.h"
#include "cert.h"
#include "ubus.h"
#include "clog.h"

#include <cJSON.h>
#include <assert.h>
#include <tuya_log.h>
#include <tuya_error_code.h>
#include <system_interface.h>
#include <mqtt_client_interface.h>
#include <tuya_endpoint.h>
#include <stdlib.h>


/*---*/

struct message_fields {
    char code[128];
    char port[128];
    int pin;
    char sensor[128];
    char model[128];
};

void on_disconnect(tuya_mqtt_context_t* context, void* user_data);
void on_messages(tuya_mqtt_context_t* context, void* user_data, const tuyalink_message_t* msg);
void on_connected(tuya_mqtt_context_t* context, void* user_data);

static inline void insert_object_data(cJSON* parent, char* field, char* output);
static int parse_message(char* input, struct message_fields* fields);
static void handle_ubus(struct message_fields* fields);

int init_tuya(char* secret, char* id, tuya_mqtt_context_t* client);
int connect_to_tuya(tuya_mqtt_context_t* client);
void cleanup_tuya(tuya_mqtt_context_t* client);

static struct ubus_context* ctx;
static struct blob_buf b;

static char* device_id;

/*---*/

static inline void insert_object_data(cJSON* parent, char* field, char* output) {
    cJSON* child = cJSON_GetObjectItem(parent, field);

    if (cJSON_IsString(child) && (child->valuestring != NULL)) {
        strncpy(output, child->valuestring, 128);
    }
}

static void handle_ubus(struct message_fields* fields) {
    if (fields == NULL) {return;}

    if (strcmp(fields->code, "devices") == 0) {
        call_devices(ctx, &b);
        return;
    }

    if (strcmp(fields->code, "off") == 0){
        call_on_off(ctx, &b, fields->port, fields->pin, 0);
        return;
    }

    if (strcmp(fields->code, "on") == 0){
        call_on_off(ctx, &b, fields->port, fields->pin, 1);
        return;
    }

    if (strcmp(fields->code, "get") == 0){
        call_get(ctx, &b, fields->port, fields->sensor, fields->pin, fields->model);
        return;
    }
}

static int parse_message(char* input, struct message_fields* fields){
    if (fields == NULL) {
        return -1;
    }

    cJSON *json = cJSON_Parse(input);
    if (json == NULL) {
        create_log("Failed to parse json", NULL);
        return -1;
    }


    cJSON *action_code = cJSON_GetObjectItem(json, "actionCode");
    if (cJSON_IsString(action_code) && action_code->valuestring == NULL) {
        cJSON_Delete(json);
        return -1;
    }

    strncpy(fields->code, action_code->valuestring, 128);

    
    cJSON *params = cJSON_GetObjectItem(json, "inputParams");
    insert_object_data(params, "port", fields->port);
    insert_object_data(params, "sensor", fields->sensor);
    insert_object_data(params, "model", fields->model);

    cJSON* pin = cJSON_GetObjectItem(params, "pin");
    if (cJSON_IsNumber(pin)) {
        fields->pin = pin->valueint;
    }

    cJSON_Delete(json);
    return 0;
}


void on_disconnect(tuya_mqtt_context_t* context, void* user_data){
    (void) context;
    (void) user_data;

    ubus_free(ctx);

    create_log("Disconnected from tuya", NULL);
}


void on_messages(tuya_mqtt_context_t* context, void* user_data, const tuyalink_message_t* msg)
{
    (void) context;
    (void) user_data;

    if (ctx == NULL) {
        return;
    }

    TY_LOGI("on message id:%s, type:%d, code:%d", msg->msgid, msg->type, msg->code);
    switch(msg->type) {
        case THING_TYPE_ACTION_EXECUTE:;
            struct message_fields fields = {.code = "", .model = "", .pin = 0, .port = "", .sensor = ""};
            parse_message(msg->data_string, &fields);
            handle_ubus(&fields);
            char* tmp = blobmsg_format_json(b.head, 1);
            tuyalink_thing_property_report(context, device_id, tmp);
            blob_buf_free(&b);
            free(tmp);
            create_log("Action Execute:", fields.code);
        default:
            break;
    }
}

void on_connected(tuya_mqtt_context_t* context, void* user_data)
{
    (void) user_data;
    (void) context;

    int result = connect_to_ubus(&ctx);
    if (result != 0) {
        create_log("Failed to connect to ubus", NULL);
    }

    tuya_endpoint_region_regist_set("AY","pro");
    system_sleep(2000);
    create_log("Connected to tuya", NULL);
}

int init_tuya(char* secret, char* id, tuya_mqtt_context_t* client){
    int ret = OPRT_OK;
    device_id = id;

    ret = tuya_mqtt_init(client, &(const tuya_mqtt_config_t) {
        .host = "m1.tuyacn.com",
        .port = 8883,
        .cacert = tuya_cacert_pem,
        .cacert_len = sizeof(tuya_cacert_pem),
        .device_id = id,
        .device_secret = secret,
        .keepalive = 60,
        .timeout_ms = 2000,
        .on_connected = on_connected,
        .on_disconnect = on_disconnect,
        .on_messages = on_messages
    });
    if (ret != OPRT_OK){
        return -1;
    }

    return 0;
}

int connect_to_tuya(tuya_mqtt_context_t* client){
    int ret = OPRT_OK;
    
    ret = tuya_mqtt_connect(client);
    if (ret != OPRT_OK){
        tuya_mqtt_deinit(client);
        return -1;
    }

    return 0;
}

void cleanup_tuya(tuya_mqtt_context_t* client){
    tuya_mqtt_disconnect(client);
    tuya_mqtt_deinit(client);
    blob_buf_free(&b);
}