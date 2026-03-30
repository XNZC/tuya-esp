#include <argp.h>
#include <string.h>
#include "clog.h"

struct login_data {
    char *device_id;
    char *product_id;
    char *secret;
};

static struct argp_option options[] =
{
    { "id", 'i', "ID", 0, "Input device id", 0},
    { "prid", 'p', "ID", 0, "Input product id", 0},
    { "secret", 's', "SECRET", 0, "Input device secret", 0},
    { 0 }
};

static int parse_opt (int key, char *arg, struct argp_state *state) {
    struct login_data* data = state->input;

    switch (key) {
        case 'i': data->device_id = arg; break;
        case 'p': data->product_id = arg; break;
        case 's': data->secret = arg; break;
    }
    
    return 0;
}

static struct argp argp = { options, parse_opt, 0, 0, 0, 0, 0 };

int take_args(int argc, char** argv, struct login_data* login_data){
    error_t err = argp_parse (&argp, argc, argv, 0, 0, login_data);

    if (err == 0) {
        return 0;
    }

    create_log("ARGP Error: ", strerror(errno));
    return -1;
}
