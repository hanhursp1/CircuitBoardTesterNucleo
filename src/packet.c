
#include "packet.h"



bool packet_check(Packet p) {
	return packet_crc(p) == p.crc;
}

uint32_t packet_crc(Packet p) {
	return crc32((uint8_t*)&p, sizeof(p) - sizeof(p.crc));
}

// Implementation taken from: https://en.wikipedia.org/wiki/Computation_of_cyclic_redundancy_checks#CRC-32_example
// I would work on my own from-scratch implementation, but we're 4 weeks out and this needs solving NOW
// But I did write it down myself so I end up understanding it somewhat.

volatile static uint32_t crc_table[256];
const uint32_t CRC32 = 0xedb88320;

void crc_init() {
	uint32_t crc32 = 1;

	for (size_t i = 128; i != 0; i >>= 1) {
		// I can kinda recognize what's going on here, but it's mostly magic to me.
		crc32 = (crc32 >> 1) ^ (crc32 & 1 ? CRC32 : 0);
		for (size_t j = 0; j < 256; j += 2*i) {
			crc_table[i + j] = crc32 ^ crc_table[j];
		}
	}
}


uint32_t crc32(const uint8_t* data, size_t size) {
	uint32_t crc = ~0;
	
	for (size_t i = 0; i < size; i++) {
		crc ^= data[i];
		crc = (crc >> 8) ^ crc_table[crc & 0xFF];
	}

	return ~crc;
}