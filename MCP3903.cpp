#include "MCP3903.h"
#include <SPI.h>


//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// PIN CONFIGURATION
MCP3903::MCP3903()
{
    pinMode(pinMOSI, OUTPUT);
    pinMode(pinMISO, INPUT);
    pinMode(pinSPIClock, OUTPUT);
    pinMode(pinCS, OUTPUT);
    digitalWrite(pinCS, HIGH);

}

//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Reset (set all values to default position)
void MCP3903::reset()
{
    init_config(o64, p1, 0, 0, 1, 1, 1, 1, 1, 1, 1);
    init_status(no, 16, 0, lag, lag, lag);
}

//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// CONFIGURATION REGISTER (11 INPUT)
void MCP3903::init_config(byte osr, byte ps, byte E_vref, byte E_clk, byte dither, byte ch0, byte ch1, byte ch2, byte ch3, byte ch4, byte ch5)
{
    unsigned long cmd1 = 0xfc0fd0; // 111111(reset) 000000(shutdown) 111111(dithering) 01(osr) 00(PS) 0(E_vref) 0(E_clk)
    unsigned long cmd2 = 0x000fc0 | (osr << 4) | (ps << 2); // 000000 000000 111111 00 00 0 0  | (osr << 4) | ps<<2

    if (E_vref == 1) { cmd2 |= 0x1 << 1; }
    if (E_clk == 1) { cmd2 |= 0x1; }
    if (dither == 0) { cmd2 &= 0xfff03f; }  // 111111 111111 000000 11 11111

    if (ch0 == 0) { cmd2 |= 0x1 << 12; }
    if (ch1 == 0) { cmd2 |= 0x1 << 13; }
    if (ch2 == 0) { cmd2 |= 0x1 << 14; }
    if (ch3 == 0) { cmd2 |= 0x1 << 15; }
    if (ch4 == 0) { cmd2 |= 0x1 << 16; }
    if (ch5 == 0) { cmd2 |= 0x1 << 17; }

    writeRegister(a_config, cmd1);
    writeRegister(a_config, cmd2);
}
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// STATUS REGISTER (6 INPUTS)
void MCP3903::init_status(byte loop, byte width, byte link, byte DRA, byte DRB, byte DRC)
{
    unsigned long cmd3 = 0x00203f | (loop << 22) | (DRA << 6) | (DRB << 8) | (DRC << 10);
    if (width == 24) { cmd3 |= 0x3f << 15; }
    if (link == 1) { cmd3 |= 1 << 12; }
    writeRegister(a_status, cmd3);
}
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// ADC READ (2 INPUTS)
double MCP3903::readADC(byte channel)
{
    // Wait for the specific bit of the status register to become 0
    while (readRegister(a_status) & (1 << channel)){}

    // Now that the bit is 0, read the ADC value
    if ( readRegister(a_status) & (1<<15) )   // For 24 bits
    {
        long r = (long) readRegister(channel);

        if (r > 8388607) {r -= 16777216l;}
        return ((double) r/8388608.0/3.000000)*2.370000;
    }


    else   // For 16 bits
    {
        byte cmdByte = a_dev | (channel << 1) | 1;
        long r=0;

        digitalWrite(pinCS, LOW);
        SPI.transfer(cmdByte);
        r |= (long)SPI.transfer(0x00) << 8;
        r |= (long)SPI.transfer(0x00);
        digitalWrite(pinCS, HIGH);

        if (r > 32767) {r -= 65536l;}
        return ((double) r/32768.0/3.000000)*2.370000;
    }

}
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//read from 2 adc
void MCP3903::read2ADC(byte channel1, byte channel2, double &result1, double &result2)
{
    // Wait for the specific bit of the status register to become 0 for channel 1
    while (readRegister(a_status) & (1 << channel1)) {}

    if ( readRegister(a_status) & (1<<15) )  // for 24 bits
    {
        byte cmdByte = a_dev | (channel1 << 1) | 1;
        long r1 = 0;
        long r2 = 0;

        digitalWrite(pinCS, LOW);
        SPI.transfer(cmdByte);
        // Read ADC value for channel 1
        r1 = (long)SPI.transfer(0x00) << 16;
        r1 |= (long)SPI.transfer(0x00) << 8;
        r1 |= (long)SPI.transfer(0x00);
        // Reade for channel 2
        r2 = (long)SPI.transfer(0x00) << 16;
        r2 |= (long)SPI.transfer(0x00) << 8;
        r2 |= (long)SPI.transfer(0x00);
        digitalWrite(pinCS, HIGH);

        // Handle 24-bit two's complement to signed conversion
        if (r1 > 8388607) {r1 -= 16777216L;}
        if (r2 > 8388607) {r2 -= 16777216L;}

        // Normalize both ADC values and store them in result 1 and result 2
        result1 = ((double)r1/(8388608.0*3.000000))*2.37000 ;
        result2 = ((double)r2/(8388608.0*3.000000))*2.37000 ;
    }

    else
    {
        byte cmdByte = a_dev | (channel1 << 1) | 1;
        long r1 = 0;
        long r2 = 0;

        digitalWrite(pinCS, LOW);
        SPI.transfer(cmdByte);
        // Read ADC value for channel1
        r1 |= (long)SPI.transfer(0x00) << 8;
        r1 |= (long)SPI.transfer(0x00);
        // Reade for channel2
        r2 |= (long)SPI.transfer(0x00) << 8;
        r2 |= (long)SPI.transfer(0x00);
        digitalWrite(pinCS, HIGH);

        // Handle 24-bit two's complement to signed conversion
        if (r1 > 32767) {r1 -= 655361L;}
        if (r2 > 32767) {r2 -= 655361L;}

        // Normalize both ADC values and store them in result1 and result2
        result1 = ((double)r1/(32768.0*3.000000))*2.37000 ;
        result2 = ((double)r2/(32768.0*3.000000))*2.37000 ;

    }
}

//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//read from 6 adc
void MCP3903::read6ADC(double &result1, double &result2, double &result3, double &result4, double &result5, double &result6)
{
    // Wait for the specific bit of the status register to become 0 for channel 1
    while (readRegister(a_status) & (1 << 0)) {}  // 0 means channel 1

    if ( readRegister(a_status) & (1<<15) )  // for 24 bits
    {
        byte cmdByte = a_dev | (0 << 1) | 1;
        long r1 = 0;
        long r2 = 0;
        long r3 = 0;
        long r4 = 0;
        long r5 = 0;
        long r6 = 0;

        digitalWrite(pinCS, LOW);
        SPI.transfer(cmdByte);
        // Read ADC value for channel 1
        r1 = (long)SPI.transfer(0x00) << 16;
        r1 |= (long)SPI.transfer(0x00) << 8;
        r1 |= (long)SPI.transfer(0x00);
        // Reade for channel 2
        r2 = (long)SPI.transfer(0x00) << 16;
        r2 |= (long)SPI.transfer(0x00) << 8;
        r2 |= (long)SPI.transfer(0x00);
        // Reade for channel 3
        r3 = (long)SPI.transfer(0x00) << 16;
        r3 |= (long)SPI.transfer(0x00) << 8;
        r3 |= (long)SPI.transfer(0x00);
        // Reade for channel 4
        r4 = (long)SPI.transfer(0x00) << 16;
        r4 |= (long)SPI.transfer(0x00) << 8;
        r4 |= (long)SPI.transfer(0x00);
        // Reade for channel 5
        r5 = (long)SPI.transfer(0x00) << 16;
        r5 |= (long)SPI.transfer(0x00) << 8;
        r5 |= (long)SPI.transfer(0x00);
        // Reade for channel 6
        r6 = (long)SPI.transfer(0x00) << 16;
        r6 |= (long)SPI.transfer(0x00) << 8;
        r6 |= (long)SPI.transfer(0x00);
        digitalWrite(pinCS, HIGH);

        // Handle 24-bit two's complement to signed conversion
        if (r1 > 8388607) {r1 -= 16777216L;}
        if (r2 > 8388607) {r2 -= 16777216L;}
        if (r3 > 8388607) {r3 -= 16777216L;}
        if (r4 > 8388607) {r4 -= 16777216L;}
        if (r5 > 8388607) {r5 -= 16777216L;}
        if (r6 > 8388607) {r6 -= 16777216L;}

        // Normalize both ADC values and store them in result 1 and result 2
        result1 = ((double)r1/(8388608.0*3.000000))*2.37000 ;
        result2 = ((double)r2/(8388608.0*3.000000))*2.37000 ;
        result3 = ((double)r3/(8388608.0*3.000000))*2.37000 ;
        result4 = ((double)r4/(8388608.0*3.000000))*2.37000 ;
        result5 = ((double)r5/(8388608.0*3.000000))*2.37000 ;
        result6 = ((double)r6/(8388608.0*3.000000))*2.37000 ;
    }

    else
    {
        byte cmdByte = a_dev | (0 << 1) | 1;
        long r1 = 0;
        long r2 = 0;
        long r3 = 0;
        long r4 = 0;
        long r5 = 0;
        long r6 = 0;

        digitalWrite(pinCS, LOW);
        SPI.transfer(cmdByte);
        // Read ADC value for channel1
        r1 |= (long)SPI.transfer(0x00) << 8;
        r1 |= (long)SPI.transfer(0x00);
        // Read for channel2
        r2 |= (long)SPI.transfer(0x00) << 8;
        r2 |= (long)SPI.transfer(0x00);
        // Read for channel3
        r3 |= (long)SPI.transfer(0x00) << 8;
        r3 |= (long)SPI.transfer(0x00);
        // Read for channel4
        r4 |= (long)SPI.transfer(0x00) << 8;
        r4 |= (long)SPI.transfer(0x00);
        // Read for channel5
        r5 |= (long)SPI.transfer(0x00) << 8;
        r5 |= (long)SPI.transfer(0x00);
        // Read for channel6
        r6 |= (long)SPI.transfer(0x00) << 8;
        r6 |= (long)SPI.transfer(0x00);
        digitalWrite(pinCS, HIGH);

        // Handle 24-bit two's complement to signed conversion
        if (r1 > 32767) {r1 -= 655361L;}
        if (r2 > 32767) {r2 -= 655361L;}
        if (r3 > 32767) {r3 -= 655361L;}
        if (r4 > 32767) {r4 -= 655361L;}
        if (r5 > 32767) {r5 -= 655361L;}
        if (r6 > 32767) {r6 -= 655361L;}


        // Normalize both ADC values and store them in result1 and result2
        result1 = ((double)r1/(32768.0*3.000000))*2.37000 ;
        result2 = ((double)r2/(32768.0*3.000000))*2.37000 ;
        result3 = ((double)r3/(32768.0*3.000000))*2.37000 ;
        result4 = ((double)r4/(32768.0*3.000000))*2.37000 ;
        result5 = ((double)r5/(32768.0*3.000000))*2.37000 ;
        result6 = ((double)r6/(32768.0*3.000000))*2.37000 ;

    }
}



//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// GAIN (2 INPUTS)
void MCP3903::Gain(byte channel, byte gain)
{
    unsigned long r = readRegister(a_gain);

    byte idx = channel * 4;
    unsigned long chGain = 0;

    if (channel % 2 == 0) // 0, 2, 4
    {
        chGain = gain ;
    }
    else // 1, 3, 5
    {
        chGain = gain << 1;
    }

    r &= ~(0xf << idx);
    r |= chGain << idx;

    writeRegister(a_gain, r);
}

//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// PHASE (2 INPUTS)
void MCP3903::phase(char group, byte phase)
{
    unsigned long cmd4 = readRegister(a_phase);

    if (group == 'a')
    {
        cmd4 &= 0xffff00;
        cmd4 |= phase;
    }

    if (group == 'b')
    {
        cmd4 &= 0xff00ff;
        cmd4 |= phase << 8;
    }

    if (group == 'c')
    {
        cmd4 &= 0x00ffff;
        cmd4 |= phase << 16;
    }

    writeRegister(a_phase, cmd4);
}

//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//read 24 bit data to register
unsigned long MCP3903::readRegister(byte reg)
{
	byte cmdByte = a_dev | reg <<1 | 1;
	unsigned long r = 0;

	digitalWrite(pinCS, LOW);
	SPI.transfer(cmdByte);
	r = (unsigned long) SPI.transfer(0x0) << 16;
	r |= (unsigned long) SPI.transfer(0x0) << 8;
	r |= (unsigned long) SPI.transfer(0x0);
	digitalWrite(pinCS, HIGH);

	return r;
}
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//write 24 bit data to register
void MCP3903::writeRegister(byte reg, unsigned long data)
{
	byte cmdByte = a_dev | reg <<1;

	byte b2 = (data & 0xff0000) >> 16;
	byte b1 = (data & 0x00ff00) >> 8;
	byte b0 = data & 0x0000ff;

	digitalWrite(pinCS, LOW);
	SPI.transfer(cmdByte);
	SPI.transfer(b2);
	SPI.transfer(b1);
	SPI.transfer(b0);
	digitalWrite(pinCS, HIGH);
}
