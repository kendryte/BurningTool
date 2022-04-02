#include "serial.h"
#include <stdint.h>

uint32_t baudrateHighValue = 460800;

void kburnSetHighSpeedValue(uint32_t baudrate) {
	debug_trace_function("%d", baudrate);
	baudrateHighValue = baudrate;
}
uint32_t kburnGetHighSpeedValue() { return baudrateHighValue; }
