#ifndef __PROCESSOR_H__
#define __PROCESSOR_H__

#include <ledctrl.pb.h>

void process_message(uint8_t *data, size_t len, ledctrl_Response &response);

#endif