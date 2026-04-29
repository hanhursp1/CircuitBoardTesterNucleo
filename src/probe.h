#pragma once

#include "servo.h"
#include "stepper.h"
#include "stm32f446xx.h"
#include <stdint.h>

typedef struct ProbeIO {
  GPIO_TypeDef *gpio;
  uint16_t probe_pin;
  uint16_t homing_pin;
} ProbeIO;

typedef struct Probe {
  // The stepper motor that serves as the rail
  Stepper rail;
  // The servo motor that operates the probe itself
  Servo axis;
  // The probe's own IO
  ProbeIO io;
  // The side the probe is on
  Side side;
  // The absolute position of the probe head, currently, in um
  int32_t x;
  int32_t y;
} Probe;

typedef struct Bed {
  Stepper stepper;
  GPIO_TypeDef *gpio;
  uint16_t homing;
} Bed;

typedef struct ProbeSet {
  Probe left;
  Probe right;
  Bed bed;
} ProbeSet;

extern ProbeSet probes;

typedef struct ProbePosition {
  float rotation;
  int32_t position;
} ProbePosition;

void ProbeSet_init(ProbeSet *probes);

bool ProbeSet_test_continuity(ProbeSet *probes);

Stepper *ProbeSet_get_stepper_by_id(ProbeSet *probes, int id);
Servo *ProbeSet_get_servo_by_id(ProbeSet *probes, int id);

void ProbeSet_lower_bed(ProbeSet *probes);
void ProbeSet_raise_bed(ProbeSet *probes);

void Bed_raise(Bed *bed);
void Bed_lower(Bed *bed);
void Bed_home(Bed *bed);

ProbePosition Probe_calculate_position(Probe *probe, uint32_t x, uint32_t y);
// void Probe_to_location(Probe* probe, ProbePosition position);
void Probe_set_position(Probe *probe, uint32_t x, uint32_t y);

inline bool Probe_at_home(Probe *probe);

void Probe_home(Probe *probe);
void ProbeSet_home(ProbeSet *probes);
