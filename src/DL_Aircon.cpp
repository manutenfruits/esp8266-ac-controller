#include "DL_Aircon.h"

#ifndef DL_SERIAL
#define DL_SERIAL Serial
#endif

/*
 * YX1FF
 *
 * 32 Bit Value
 * NEC Encoding
 *                                     | Timer hrs |
 * +--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+
 * |  Mode  |ON| Fan | 0| S|Temperature|.5|10|20|TO|
 * +--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+
 * | Timer Hr  | 0| 1| 1| 0| 0| 0|5f|CF|Timer| 1| 0|
 * +--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+
 *
 * Mode can be one of
 * - 100 Cool
 * - 010 Dry
 * - 110 Fan
 * - 001 Heat
 * - 000 Auto
 *
 * Fan can be 0-4 (0 is auto)
 *
 * S  Sleep mode
 *
 * Temperature is 4-bit (16-30C)
 *
 * Timer is set by sending the same signal first with '10' and then '01'
 * in the 'Timer' section. The number of hours is set in Timer Hr, tick
 * '.5', '10', '20' to add that many hours. TO means timer ON.
 *
 * 5f is the bit to set the second correspondance of C to F, since it's
 * not a 1:1 situation.
 *
 * CF   Celsius/Fahrenheit setting
 *
 * Both the temperature and timer have their bit-order reversed.
 */

unsigned long dl_assemble_msg(dl_aircon_msg_t* msg){
  unsigned long buf = 0x602;

  if (!msg->unitF){
    msg->temperature = constrain(msg->temperature, 16, 30);
    buf |= bit_reverse(msg->temperature-16) << 20;
  }else{
    // msg->temperature = constrain(msg->temperature, 61, 89);
    // buf |= bit_reverse(msg->temperature);
  }

  // buf |= bit_reverse(msg->timer_value) << 8;

  if (msg->on) buf |= 0x1 << 28;
  // if (msg->timer) buf |= 0x1 << 9;
  if (msg->unitF) buf |= 0x1 << 4;

  if (msg->sleep) buf |= 0x1 << 24;

  switch(msg->mode) {
    case HEAT:  buf |= 0x1 << 29; break;
    case DRY:   buf |= 0x2 << 29; break;
    case COOL:  buf |= 0x4 << 29; break;
    case FAN:   buf |= 0x6 << 29; break;
    default:
    case AUTO:  buf |= 0x0 << 29; break;
  }

  if (msg->fan == 0 || msg->fan == 1 || msg->fan == 2 || msg->fan == 3){
    buf |= msg->fan << 26;
  }

  return buf;
}

bool dl_decode_msg(unsigned long code, dl_aircon_msg_t* msg){

  msg->on = ( (code) >> 28 ) & 0x01;
  // msg->timer = ( (code) >> 9 ) & 0x01;
  msg->sleep = (code >> 24) & 0x01;
  msg->unitF = ( (code) >> 4 ) & 0x01;

  msg->temperature = bit_reverse((code >> 20) & 0x0F) >> 4;
  if (!msg->unitF) { //degC has a -16 offset
      msg->temperature += 16;
  }

  // msg->timer_value = bit_reverse((code >> 12) & 0xF) >> 4;

  switch(code >> 29 & 0x07) {
    case 0x1: msg->mode = HEAT; break;
    case 0x2: msg->mode = DRY; break;
    case 0x4: msg->mode = COOL; break;
    case 0x6: msg->mode = FAN; break;
    default:
    case 0x0: msg->mode = AUTO; break;
  }

  msg->fan = bit_reverse((code >> 26) & 0x03) >> 6;
}

void dl_print_msg(dl_aircon_msg_t *msg){
  DL_SERIAL.println("Settings:");

  if (msg->on){
    DL_SERIAL.print("ON  ");
  }else{
    DL_SERIAL.print("OFF ");
  }

  DL_SERIAL.print(msg->temperature);

  if (msg->unitF){
    DL_SERIAL.println(" degF");
  }else{
    DL_SERIAL.println(" degC");
  }

  switch (msg->mode){
    case HEAT:
      DL_SERIAL.print("Heat ");
      break;
    case DRY:
      DL_SERIAL.print("Dry  ");
      break;
    case COOL:
      DL_SERIAL.print("Cool ");
      break;
    case FAN:
      DL_SERIAL.print("Fan  ");
      break;
    case AUTO:
      DL_SERIAL.print("Auto ");
      break;
  }

  switch (msg->fan){
    case 0:
      DL_SERIAL.println("Auto ");
      break;
    case 1:
      DL_SERIAL.println("Low  ");
      break;
    case 2:
      DL_SERIAL.println("Mid  ");
      break;
    case 3:
      DL_SERIAL.println("High ");
      break;
  }

  if (msg->sleep) {
    DL_SERIAL.println("Sleep ON");
  } else {
    DL_SERIAL.println("Sleep OFF");
  }

  // if (msg->timer){
  //   DL_SERIAL.print("Timer On:  ");
  // }else{
  //   DL_SERIAL.print("Timer Off: ");
  // }

  // DL_SERIAL.println(msg->timer_value);

  DL_SERIAL.println(" ");
}

// Reverse the order of bits in a byte.
// I.e. MSB is swapped with LSB, etc.
unsigned char bit_reverse( unsigned char x )
{
   x = ((x >> 1) & 0x55) | ((x << 1) & 0xaa);
   x = ((x >> 2) & 0x33) | ((x << 2) & 0xcc);
   x = ((x >> 4) & 0x0f) | ((x << 4) & 0xf0);
   return x;
}
