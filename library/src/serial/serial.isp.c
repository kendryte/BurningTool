#include "isp.h"

size_t serial_isp_packet_size(const isp_request_t *packet) {
	// debug_print("ISP_HEADER_SIZE=%ld", offsetof(isp_request_t, data));
	// debug_print("packet->data_len=%d", packet->data_len);
	return offsetof(isp_request_t, data) + packet->data_len;
}

static uint32_t checksum(const isp_request_t *packet) {
	// debug_print("[checksum] data_len=%d, :data=%ld, :address=%ld", packet->data_len, offsetof(isp_request_t, data), offsetof(isp_request_t,
	// address));
	size_t size = packet->data_len + offsetof(isp_request_t, data) - offsetof(isp_request_t, address);
	uint8_t *message = (void *)&packet->address;

	uint32_t crc = 0xFFFFFFFF;
	while (size != 0) {
		uint32_t byte = *message;
		crc = crc ^ byte;
		for (int j = 7; j >= 0; j--) {
			uint32_t mask = -(crc & 1);
			crc = (crc >> 1) ^ (0xEDB88320 & mask);
		}

		message++;
		size--;
	}
	return ~crc;
}

/** unused */
bool serial_isp_verify_checksum(isp_request_t *packet) {
	return checksum(packet) == packet->checksum;
}

void serial_isp_calculate_checksum(isp_request_t *packet) {
	if (packet->checksum == 0) {
		packet->checksum = checksum(packet);
	}
}
