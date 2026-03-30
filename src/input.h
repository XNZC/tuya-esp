#ifndef INPUT_H
#define INPUT_H

#include <stdlib.h>

struct login_data {
    char *device_id;
    char *product_id;
    char *secret;
};

int take_args(int argc, char** argv, struct login_data* login_data);

#endif
