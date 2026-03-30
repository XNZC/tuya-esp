#ifndef UBUS_H
#define UBUS_H

#include <libubus.h>
#include <libubox/blobmsg_json.h>

int connect_to_ubus(struct ubus_context** ctx);
int call_devices(struct ubus_context* ctx, struct blob_buf* buf);
int call_get(struct ubus_context* ctx, struct blob_buf* buf, char* port, char* sensor, int pin, char* model);
int call_on_off(struct ubus_context* ctx, struct blob_buf* buf, char* port, int pin, int select);

#endif /*UBUS_H*/