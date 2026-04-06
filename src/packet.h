#pragma once

#include "common.h"
#include "netlist.h"
#include <stdint.h>

typedef enum PacketKind {
	// Response
	PK_Poke = 0,
	// Point data
	PK_Point,
	// Net data
	PK_Entry,
	// Resend request
	PK_Resend,
	// Point read result
	PK_Result,
} PacketKind;

typedef struct Packet {
	PacketKind kind;
	union {
		struct {
			uint32_t id;
			NetlistPoint p;
		} point;
		struct {
			uint32_t id;
			NetlistEntry v;
		} vert;
		struct {
			uint32_t n_id;
			uint32_t p_id;
			bool pass;
		} result;
	} data;
	uint32_t crc;
} Packet;


bool packet_check(Packet p);
uint32_t packet_crc(Packet p);

void crc_init();
uint32_t crc32(const uint8_t* data, size_t size);