#ifndef __CMD_PROCESSOR_H__
#define __CMD_PROCESSOR_H__

#include <pi.pb.h>

void process_message(const uint8_t *data, size_t len, piproto_Response &response);

size_t encode_response(const piproto_Response &response, uint8_t *buffer, int buflen);

#endif