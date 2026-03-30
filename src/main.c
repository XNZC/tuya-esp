#include "clog.h"
#include "input.h"
#include "tuya.h"

#include <signal.h>

static int rc = 0;
static void signal_handler(){
    rc = 1;
}

static inline void get_params(struct login_data* data, int argc, char** argv) {
    int result = take_args(argc, argv, data);
    if (result == -1 || data->device_id == NULL || data->product_id == NULL || data->secret == NULL) {
        create_log("Input is invalid", NULL);
        exit(EXIT_FAILURE);
    }
}

static inline void init(struct login_data* data, tuya_mqtt_context_t* client) {
    int result = init_tuya(data->secret, data->device_id, client);
    if (result != 0) {
        create_log("Failed to init tuya", NULL);
        exit(EXIT_FAILURE);
    }
}

static inline void connect(tuya_mqtt_context_t* client){
    int result = connect_to_tuya(client);
    if (result != 0) {
        create_log("Failed to connect to tuya", NULL);
        exit(EXIT_FAILURE);
    }
}

int main(int argc, char **argv){
    signal(SIGINT, signal_handler);
    signal(SIGTERM, signal_handler);
    
    struct login_data data = { .device_id = NULL, .product_id = NULL, .secret = NULL };
    get_params(&data, argc, argv);

    tuya_mqtt_context_t client;
    init(&data, &client);

    connect(&client);

    while (rc == 0) {
        tuya_mqtt_loop(&client);
    }

    cleanup_tuya(&client);
    return 0;
}