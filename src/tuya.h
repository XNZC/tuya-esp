#ifndef TUYA_H
#define TUYA_H

#include <tuyalink_core.h>
#include <tuya_error_code.h>
#include <tuya_endpoint.h>
#include <tuya_log.h>

int init_tuya(char* secret, char* device_id, tuya_mqtt_context_t* client);
int connect_to_tuya(tuya_mqtt_context_t* client);
void cleanup_tuya(tuya_mqtt_context_t* client);

#endif /*TUYA_H*/
