#ifndef __CMD_PROCESSOR_H__
#define __CMD_PROCESSOR_H__

#include <ledctrl.pb.h>

void process_message(const uint8_t *data, size_t len, ledctrl_Response &response);

size_t encode_response(const ledctrl_Response &response, uint8_t *buffer, int buflen);

#endif