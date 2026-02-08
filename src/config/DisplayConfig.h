#ifndef DISPLAYCONFIG_H
#define DISPLAYCONFIG_H

// Region definitions for display layout
struct Region {
  int16_t x, y, w, h;
};

// Widget IDs
enum WidgetID : uint8_t {
  W_LEAN = 0,
  W_TIME,
  W_SPEED,
  W_INFO,
  W_AMBI,
  W_HUMID,
  W_TIRE_L,
  W_ENGINE,
  W_TIRE_R,
  W__COUNT
};


#endif