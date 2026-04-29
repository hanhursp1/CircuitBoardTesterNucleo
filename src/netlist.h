#pragma once

#include <stdint.h>
#include <stdio.h>
#include "config.h"
#include "usart.h"

typedef struct NetlistEntry {
	uint16_t start_index;
	uint16_t length;
} NetlistEntry;

typedef struct NetlistPoint {
	int32_t x;
	int32_t y;
} NetlistPoint;

typedef struct NetlistEntryInfo {
	uint16_t  length;
	NetlistPoint* start_point;
} NetlistEntryInfo;

extern NetlistPoint points_buffer[MAX_POINTS];
extern NetlistEntry nets_buffer[MAX_INDICES];

// Get the net at the point as a pair struct representing a length and indexable sub-array
NetlistEntry get_net(int id);
NetlistPoint get_point(int id);
int get_net_count();
int get_vert_count();

void set_vert_count(int cnt);

void set_net_count(int cnt);

void clear_netlist();

void set_vert(int id, int32_t x, int32_t y);
void set_net(int id, uint16_t start, uint16_t len);

// Slices a `NetlistEntryInfo` object, returning a subsection of it
NetlistEntryInfo NetlistEntryInfo_slice(NetlistEntryInfo self, int from, int to);