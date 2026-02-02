#include "netlist.h"
#include "config.h"
#include <assert.h>
#include <stdio.h>

int num_points = 0;
NetlistPoint points_buffer[MAX_POINTS] = {0};

int num_nets = 0;
NetlistEntry nets_buffer[MAX_INDICES] = {0};

NetlistEntryInfo get_net(int id) {
	NetlistEntry n = nets_buffer[id];
	NetlistEntryInfo result = {.length= n.length, .start_point= &points_buffer[n.start_index]};
	return result;
}

int get_net_count() {
	return num_nets;
}

void clear_netlist() {
	num_nets = 0;
	num_points = 0;
}

void fill_netlist_from_file(FILE* f) {
	// TODO: Make this a byte stream instead of ascii text
	fflush(f);
	fscanf(f, "%d", &num_nets);
	// TODO: Panic if num_nets is greater than MAX_INDICES
	for (int i = 0; i < num_nets; i++) {
		int start, len;
		fscanf(f, "%d,%d", &start, &len);
		nets_buffer[i].length = len;
		nets_buffer[i].start_index = start;
	}
}

void fill_pointlist_from_file(FILE *f) {
	// TODO: Make this a byte stream instead of ascii text.
	fflush(f);
	fscanf(f, "%d", &num_points);
	// TODO: Panic if num_points is greater than MAX_POINTS
	for (int i = 0; i < num_points; i++) {
		int x, y;
		fscanf(f, "%d,%d", &x, &y);
		points_buffer[i].x = x;
		points_buffer[i].y = y;
	}
}

NetlistEntryInfo NetlistEntryInfo_slice(NetlistEntryInfo self, int from, int to) {
	// Assert that the new endpoint is within bounds
	assert(to <= self.length);
	// Set the new start point and length, return the modified object
	self.start_point += from;
	self.length = to - from;
	return self;
}

void NetlistEntryInfo_probe(NetlistEntryInfo self) {
	while (self.length >= MIN_NET_SIZE) {
		int num_bad = 0;
		// The static point of the net
		NetlistPoint static_point = self.start_point[0];
		for (int i = 1; i < self.length; i++) {
			// The dynamic point that we're testing
			NetlistPoint dynamic_point = self.start_point[i];
			// TODO: Test between the static and dynamic points, record and report the results
			num_bad += 1;
		}
		// All test points failed, meaning the static point is likely bad. Report this and start from the next point!
		if (num_bad == self.length) {
			self = NetlistEntryInfo_slice(self, 1, self.length);
		} 
		// Else, at least one connection was established, report all failures and exit.
		else {
			break;
		}
	}
}