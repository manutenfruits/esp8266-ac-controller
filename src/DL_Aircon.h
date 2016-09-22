#include <arduino.h>

#ifndef DL_Aircon_h
#define DL_Aircon_h

enum Mode { COOL, DRY, FAN, HEAT, AUTO };

typedef struct {
  bool on;
  Mode mode;
  int fan;
  bool sleep;
  int temperature;
  // bool timer;
  // int timer_value;
  bool unitF;
} dl_aircon_msg_t;

unsigned long dl_assemble_msg(dl_aircon_msg_t* msg);
bool dl_decode_msg(unsigned long code, dl_aircon_msg_t* msg);
void dl_print_msg(dl_aircon_msg_t *msg);
unsigned char bit_reverse( unsigned char x );


#endif
