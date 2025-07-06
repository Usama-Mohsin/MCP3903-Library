#ifndef MCP3903_H
#define MCP3903_H
#include <Arduino.h>

class MCP3903 {
public:
  // REGISTER ADDRESSES (TOTAL 11) //
  static const byte a_dev = 0x40;

 // TYPE 1 : CHANNEL ADDRESSES CHANNEL 0 HAS 0 ADDRESS SO NO NEED TO DEFINE JUST WRITE 0 (SAME FOR ALL 6 CHANNELS)

  // TYPE 2
  static const byte a_mod = 0x06;    // GROUP 4
  static const byte a_phase = 0x07;  // GROUP 4
  static const byte a_gain = 0x08;   // GROUP 4

  static const byte a_status = 0x09;  // GROUP 5
  static const byte a_config = 0x0A;  // GROUP 5

  // GAINS
  static const byte g1 = 0x0;  // 000
  static const byte g2 = 0x1;  // 001
  static const byte g4 = 0x2;  // 010
  static const byte g8 = 0x3;  // 011
  static const byte g16 = 0x4; // 100
  static const byte g32 = 0x5; // 101

  // OSR
  static const byte o32 = 0x0;  // 00
  static const byte o64 = 0x1;  // 01
  static const byte o128 = 0x2; // 10
  static const byte o256 = 0x3; // 11

  // Pre Scaler
  static const byte p1 = 0x0;  // 00
  static const byte p2 = 0x1;  // 01
  static const byte p4 = 0x2;  // 10
  static const byte p8 = 0x3;  // 11

  // DR_n mode
  static const byte lag = 0x0;   // 00
  static const byte first = 0x1; // 01
  static const byte second = 0x2;// 10
  static const byte both = 0x3;  // 11

  // READ(loop)
  static const byte no = 0x0;   // 00
  static const byte group = 0x1; // 01
  static const byte type = 0x2;  // 10
  static const byte all = 0x3;   // 11

  // Functions
  MCP3903();
  void reset();
  unsigned long readRegister(byte reg);
  void writeRegister(byte reg, unsigned long data);
  void init_config(byte osr, byte ps, byte E_vref, byte E_clk, byte dither, byte ch0, byte ch1, byte ch2, byte ch3, byte ch4, byte ch5);
  void init_status(byte loop, byte width, byte link, byte DRA, byte DRB, byte DRC);
  double readADC(byte channel);
  void read2ADC(byte channel1, byte channel2, double &result1, double &result2);
  void read6ADC(double &result1, double &result2, double &result3, double &result4, double &result5, double &result6);
  void Gain(byte channel, byte gain);
  void phase(char group, byte phase);

private:
  static const int pinMOSI = 23; // MOSI
  static const int pinMISO = 19; // MISO
  static const int pinSPIClock = 18; // SCK
  static const int pinCS = 5; // CS
};
#endif
