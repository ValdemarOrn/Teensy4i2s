#include "Arduino.h"
#include "Wire.h"
#include "control_TLV320AIC3204.h"

AudioControlTLV320AIC3204::AudioControlTLV320AIC3204(uint8_t address)
{
    this->address = address;
    Wire.begin();
    Wire.setClock(400000);
}

void AudioControlTLV320AIC3204::init()
{
    // ----------------- PAGE 0 -----------------

    // example frequencies:
    // fs (samplerate): 48khz
    // WCLK - Word Clock - fs = 48Khz
    // BCLK - Bit Block - 3.072Mhz  (64*fs)
    // MCLK - Master Clock - 12.288Mhz  (256*fs)

    reset();

    // Audio Interface Setting Register 3
    disableLoopback();

    // 11: Clock Setting Register 6, NDAC Values - 0x00 / 0x0B
    writeRegister(0x0B, 0, 0b10000001); // power on NDAC, value = 1

    // 12: Clock Setting Register 7, MDAC Values - 0x00 / 0x0C
    writeRegister(0x0C, 0, 0b10000010); // power on MDAC, value = 2 - this gives us the correct ADC/DAC frequency for MCLK = 256*fs, which is what the Teensy will send.

    // 13: DAC OSR Setting Register 1, MSB Value - 0x00 / 0x0D
    writeRegister(0x0D, 0, 0); // OSR top bits

    // 14: DAC OSR Setting Register 2, LSB Value - 0x00 / 0x0E
    writeRegister(0x0E, 0, 128); // OSR = divide by 128

    // 18: Clock Setting Register 8, NADC Values - 0x00 / 0x12
    writeRegister(0x12, 0, 0b10000001); // power on NADC, value = 1

    // 19: Clock Setting Register 9, MADC Values - 0x00 / 0x13
    writeRegister(0x13, 0, 0b10000010); // power on MADC, value = 2

    // 20: ADC Oversampling (AOSR) Register - 0x00 / 0x14
    writeRegister(0x14, 0, 128); // OSR = divide by 128

    // 27: Audio Interface Setting Register 1 - 0x00 / 0x1B
    writeRegister(0x1B, 0, 0b00110000); // use I2S, 32 bit samples, BCLK and WCLK and inputs. 20+ bits requires PTM_P4 (powerTune mode 4)

    // 60: DAC Signal Processing Block Control Register - 0x00 / 0x3C
    writeRegister(0x3C, 0, 0x08); // DAC Mode to PRB_P8

    // 61: ADC Signal Processing Block Control Register - 0x00 / 0x3D
    writeRegister(0x3D, 0, 0x01); // ADC Mode to PRB_P1 -- should probably be P8!

    // Enable DAC -- happens at bottom of function

    dacVolume(0, 0);

    // 68: DRC Control Register 1
    writeRegister(0x44, 0, 0b00000000); // Disable dynamic range compression

    // 81: ADC Channel Setup Register
    writeRegister(0x51, 0, 0b11000000); // ADC power

    // 82: ADC Fine Gain Adjust Register
    writeRegister(0x52, 0, 0b00000000); // ADC unmute, set fine gain control to 0db

    adcGain(0, 0);

    // ----------------- PAGE 1 -----------------

    // 1: Power Configuration Register
    writeRegister(0x01, 1, 0b00001000); // disable weak connection of AVDD to DVDD, since we're using the AVDD LDO

    // 2: LDO Control Register - 0x01 / 0x02
    writeRegister(0x02, 1, 0b00000001); // Analog blocks powered up, AVDD LDO powered up // D3 is suspicious, because datasheet says it's a zero for enabled. could be wrong?

    // 3: Playback Configuration Register 1 - 0x01 / 0x03
    writeRegister(0x03, 1, 0b0); // HP uses class-AB driver. PTM_P3/4 active

    // 4: Playback Configuration Register 2 - 0x01 / 0x04
    writeRegister(0x04, 1, 0b0); // HP uses class-AB driver. PTM_P3/4 active

    // 9: Output Driver Power Control Register - 0x01 / 0x09
    writeRegister(0x09, 1, 0b00111111); // power up HPL, HPR, LOL, LOR, Mixer amplifier MAL and MAR

    // 10: Common Mode Control Register - 0x01 / 0x0A
    writeRegister(0x0A, 1, 0b00000000); // 0.9v common mode, HP and LO use common mode

    // 12: HPL Routing Selection Register - 0x01 / 0x0C
    /*
    D7-D4 R 0000 Reserved. Write only default values
    D3 R/W 0 0: Left Channel DAC reconstruction filter's positive terminal is not routed to HPL
                1: Left Channel DAC reconstruction filter's positive terminal is routed to HPL
    D2 R/W 0 0: IN1L is not routed to HPL
                1: IN1L is routed to HPL
    D1 R/W 0 0: MAL output is not routed to HPL
                1: MAL output is routed to HPL
    D0 R/W 0 0: MAR output is not routed to HPL
                1: MAR output is routed to HPL
    */
    writeRegister(0x0C, 1, 0b00001000); // DAC Left to HPL
    //writeRegister(0x0C, 1, 0b00001100); // DAC Left and INL to HPL

    // 13: HPR Routing Selection Register - 0x01 / 0x0D
    writeRegister(0x0D, 1, 0b00001000); // DAC Right to HPR
    //writeRegister(0x0D, 1, 0b00001100); // DAC Right and INR to HPR

    // 14: LOL Routing Selection Register - 0x01 / 0x0E
    /*
    D7-D5 R 000 Reserved. Write only default values
    D4 R/W 0 0: Right Channel DAC reconstruction filter's negative terminal is not routed to LOL
                1: Right Channel DAC reconstruction filter's negative terminal is routed to LOL
    D3 R/W 0 0: Left Channel DAC reconstruction filter output is not routed to LOL
                1: Left Channel DAC reconstruction filter output is routed to LOL
    D2 R 0 Reserved. Write only default value.
    D1 R/W 0 0: MAL output is not routed to LOL
                1: MAL output is routed to LOL
    D0 R/W 0 0: LOR output is not routed to LOL
                1: LOR output is routed to LOL(use when LOL and LOR output is powered by AVDD)
    */
    writeRegister(0x0E, 1, 0b00001000); // DAC Left to LOL

    // 15: LOR Routing Selection Register - 0x01 / 0x0F
    /*
    D7-D4 R 0000 Reserved. Write only default values
    D3 R/W 0 0: Right Channel DAC reconstruction filter output is not routed to LOR
                1: Right Channel DAC reconstruction filter output is routed to LOR
    D2 R 0 Reserved. Write only default value.
    D1 R/W 0 0: MAR output is not routed to LOR
                1: MAR output is routed to LOR
    D0 R 0 Reserved. Write only default value.
    */
    writeRegister(0x0F, 1, 0b00001000); // DAC Right to LOR

    headphoneGain(0, 0, false);
    lineOutGain(0, 0, false);

    // 20: Headphone Driver Startup Control Register - 0x01 / 0x14
    writeRegister(0x14, 1, 0b00100101); // headphone soft ramp-up, 6k resistance

    // All inputs routings use 10k input resistance

    // 52: Left MICPGA Positive Terminal Input Routing Configuration Register - 0x01 / 0x34
    writeRegister(0x34, 1, 0b00010000); // IN2L to PGA positive - the routing only offers IN2L/R differential to the left channel!!

    // 54: Left MICPGA Negative Terminal Input Routing Configuration Register - 0x01 / 0x36
    writeRegister(0x36, 1, 0b00010000); // IN2R to PGA negative - the routing only offers IN2L/R differential to the left channel!!

    // 55: Right MICPGA Positive Terminal Input Routing Configuration Register - 0x01 / 0x37 
    writeRegister(0x37, 1, 0b01000000); // IN1R to PGA positive - the routing only offers inverting the polarity here!!

    // 57: Right MICPGA Negative Terminal Input Routing Configuration Register - 0x01 / 0x39 (P1_R57)
    writeRegister(0x39, 1, 0b00010000); // IN1L to PGA negative - the routing only offers inverting the polarity here!!


    //123: Reference Power-up Configuration Register - 0x01 / 0x7B
    writeRegister(0x7B, 1, 0x01); // reference power up in 40ms

    delay(100); // wait longer for soft step to take place?

    // 63: DAC Channel Setup Register 1
    writeRegister(0x3F, 0, 0b11010100); // enable DAC, send left and right channels to it
    // 64: DAC Channel Setup Register 2
    writeRegister(0x40, 0, 0b00000000); // unmute DAC
}

void AudioControlTLV320AIC3204::setPage(uint8_t page)
{
    Wire.beginTransmission(address);
    Wire.write(byte(0x00)); // address
    Wire.write(page);
    Wire.endTransmission();
}

uint8_t AudioControlTLV320AIC3204::readRegister(uint8_t reg, uint8_t page)
{
    setPage(page);
    Wire.beginTransmission(address);
    Wire.write(reg);
    Wire.endTransmission();

    Wire.requestFrom(address, 1);
    byte data = Wire.receive();
    Wire.endTransmission();
    return data;
}

void AudioControlTLV320AIC3204::writeRegister(uint8_t reg, uint8_t page, uint8_t data)
{
    setPage(page);
    Wire.beginTransmission(address);
    Wire.write(reg);
    Wire.write(data);
    Wire.endTransmission();
}

void AudioControlTLV320AIC3204::reset()
{
    // Audio Interface Setting Register 3
    writeRegister(0x01, 0, 0b00000001);
    delay(2);
}

void AudioControlTLV320AIC3204::enableLoopbackAdc()
{
    // Audio Interface Setting Register 3
    writeRegister(0x1D, 0, 0b00010000);
}

void AudioControlTLV320AIC3204::enableLoopbackI2s()
{
    // Audio Interface Setting Register 3
    writeRegister(0x1D, 0, 0b00100000);
}

void AudioControlTLV320AIC3204::disableLoopback()
{
    // Audio Interface Setting Register 3
    writeRegister(0x1D, 0, 0b00000000);
}

void AudioControlTLV320AIC3204::analogInGain(uint8_t leftValue, uint8_t rightValue)
{
    /*
    Set volume in 0.5db increments
    0 = 0db
    +95 = +47.5dB
    This is an ANALOG gain control

    Note that when using a differential signal you'll end up with +6db gain 
    at a setting of zero due to the summing of the two differential signals
    */

    if (leftValue > 95)
        leftValue = 95;
    if (rightValue > 95)
        rightValue = 95;

    // 59: Left MICPGA Volume Control Register - 0x01 / 0x3B 
    writeRegister(0x3B, 1, leftValue);

    // 60: Right MICPGA Volume Control Register - 0x01 / 0x3C (P1_R60)
    writeRegister(0x3C, 1, rightValue);
}

void AudioControlTLV320AIC3204::dacVolume(int8_t leftValue, int8_t rightValue)
{
    /*
    Set volume in 0.5db increments
    -127 = -63.5dB
    0 = 0db
    +48 = +24dB
    This is a DIGITAL volume control - pre-DAC conversion
    */

    if (leftValue > 48)
        leftValue = 48;
    if (rightValue > 48)
        rightValue = 48;

    // 65: Left DAC Channel Digital Volume Control Register
    writeRegister(0x41, 0, leftValue);
    // 66: Right DAC Channel Digital Volume Control Register
    writeRegister(0x42, 0, rightValue);
}

void AudioControlTLV320AIC3204::adcGain(int8_t leftGain, int8_t rightGain)
{
    /*
    Set gain in 0.5dB increments
    -24 = -12dB
    0 = 0db
    +40 = +20dB
    This is a DIGITAL volume control - post-ADC conversion
    */

    if (leftGain < -24)
        leftGain = -24;
    if (rightGain < -24)
        rightGain = -24;

    if (leftGain > 40)
        leftGain = 40;
    if (rightGain > 40)
        rightGain = 40;

    // 83: Left ADC Channel Volume Control Register
    writeRegister(0x53, 0, leftGain);

    // 84: Right ADC Channel Volume Control Register
    writeRegister(0x54, 0, rightGain);
}

void AudioControlTLV320AIC3204::headphoneGain(int8_t leftGain, int8_t rightGain, bool mute)
{
    /*
    Set gain in 1dB increments
    -6 = -6dB
    0 = 0db
    +29 = +29dB
    */

    if (leftGain < -6)
        leftGain = -6;
    if (rightGain < -6)
        rightGain = -6;

    if (leftGain > 29)
        leftGain = 29;
    if (rightGain > 29)
        rightGain = 29;

    leftGain &= 0b00111111;
    rightGain &= 0b00111111;

    if (mute) 
    {
        leftGain |= 0b01000000;
        rightGain |= 0b01000000;
    }

    // 16: HPL Driver Gain Setting Register - 0x01 / 0x10 (P1_R16)
    writeRegister(0x10, 1, leftGain);

    // 17: HPR Driver Gain Setting Register - 0x01 / 0x11 (P1_R17)
    writeRegister(0x11, 1, rightGain);
}


void AudioControlTLV320AIC3204::lineOutGain(int8_t leftGain, int8_t rightGain, bool mute)
{
    /*
    Set gain in 1dB increments
    -6 = -6dB
    0 = 0db
    +29 = +29dB
    */

    if (leftGain < -6)
        leftGain = -6;
    if (rightGain < -6)
        rightGain = -6;

    if (leftGain > 29)
        leftGain = 29;
    if (rightGain > 29)
        rightGain = 29;

    leftGain &= 0b00111111;
    rightGain &= 0b00111111;

    if (mute) 
    {
        leftGain |= 0b01000000;
        rightGain |= 0b01000000;
    }

    // 18: LOL Driver Gain Setting Register - 0x01 / 0x12 (P1_R18)
    writeRegister(0x12, 1, leftGain);

    // 19: LOR Driver Gain Setting Register - 0x01 / 0x13 (P1_R19)
    writeRegister(0x13, 1, rightGain);
}
