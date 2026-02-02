#pragma once

#include <stdint.h>
#include <stdio.h>
#include "config.h"

typedef struct NetlistEntry {
	uint16_t start_index;
	uint16_t length;
} NetlistEntry;

typedef struct NetlistPoint {
	int16_t x;
	int16_t y;
} NetlistPoint;

typedef struct NetlistEntryInfo {
	uint16_t  length;
	NetlistPoint* start_point;
} NetlistEntryInfo;

extern NetlistPoint points_buffer[MAX_POINTS];
extern NetlistEntry nets_buffer[MAX_INDICES];

// Get the net at the point as a pair struct representing a length and indexable sub-array
NetlistEntryInfo get_net(int id);
int get_net_count();

void clear_netlist();

void fill_netlist_from_file(FILE* f);
void fill_pointlist_from_file(FILE* f);

// Slices a `NetlistEntryInfo` object, returning a subsection of it
NetlistEntryInfo NetlistEntryInfo_slice(NetlistEntryInfo self, int from, int to);