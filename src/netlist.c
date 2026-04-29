#include "netlist.h"
#include "config.h"
#include "probe.h"
#include "stm32f4xx_hal.h"
#include "usart.h"
#include <assert.h>
#include <stdio.h>

int num_points = 0;
NetlistPoint points_buffer[MAX_POINTS] = {0};

int num_nets = 0;
NetlistEntry nets_buffer[MAX_INDICES] = {0};

NetlistEntry get_net(int id) {
	NetlistEntry n = nets_buffer[id];
	return n;
}

NetlistPoint get_point(int id) {
	NetlistPoint p = points_buffer[id];
	return p;
}

int get_net_count() {
	return num_nets;
}

int get_vert_count() {
	return num_points;
}

void set_net_count(int cnt) {
	num_nets = cnt;
}

void set_vert_count(int cnt) {
	num_points = cnt;
}

void clear_netlist() {
	num_nets = 0;
	num_points = 0;
}

void set_vert(int id, int32_t x, int32_t y) {
	points_buffer[id] = (NetlistPoint){.x = x, .y = y};
}

void set_net(int id, uint16_t start, uint16_t len) {
	nets_buffer[id] = (NetlistEntry){.start_index = start, .length = len};
}

NetlistEntry NetlistEntry_slice(NetlistEntry self, int from, int to) {
	// Assert that the new endpoint is within bounds
	assert(to <= self.length);
	// Set the new start point and length, return the modified object
	self.start_index += from;
	self.length = to - from;
	return self;
}

void NetlistEntry_probe(NetlistEntry self) {
	while (self.length >= MIN_NET_SIZE) {
		int num_bad = 0;

		// The static point of the net
		NetlistPoint static_point = get_point(self.start_index);
		Probe_set_position(&probes.right, static_point.x, static_point.y);

		for (int i = 1; i < self.length; i++) {
			int id = self.start_index + i;
			Bed_lower(&probes.bed);
			// The dynamic point that we're testing
			NetlistPoint dynamic_point = get_point(id);
			// Set the left probe position to this dynamic point
			Probe_set_position(&probes.left, dynamic_point.x, dynamic_point.y);

			Bed_raise(&probes.bed);
			HAL_Delay(10);
			bool continuity = ProbeSet_test_continuity(&probes);
			if (continuity) {
				// TODO: Report positive
				printf("!res:%d:pass;\n", id);
			} else {
				// TODO: Report negative
				printf("!res:%d:fail;\n", id);
				num_bad += 1;
			}
		}
		Bed_lower(&probes.bed);
		
		// All test points failed, meaning the static point is likely bad. Report this and start from the next point!
		if (num_bad == self.length) {
			self = NetlistEntry_slice(self, 1, self.length);
		} 
		// Else, at least one connection was established, report all failures and exit.
		else {
			break;
		}
	}
}