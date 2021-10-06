#pragma once

// https://www.ti.com/lit/ml/slaa557/slaa557.pdf  Reference Sheet
// https://www.ti.com/lit/ds/symlink/tlv320aic3204.pdf  Data Sheet
// TI TLV320AIC3204 Codec control
class AudioControlTLV320AIC3204
{
public:
    int address;
    AudioControlTLV320AIC3204(uint8_t address = 0x18);
    void init();
    void setPage(uint8_t page);
    uint8_t readRegister(uint8_t reg, uint8_t page);
    void writeRegister(uint8_t reg, uint8_t page, uint8_t data);
    void reset();
    void enableLoopbackAdc();
    void enableLoopbackI2s();
    void disableLoopback();
    void analogInGain(uint8_t leftValue, uint8_t rightValue);
    void dacVolume(int8_t leftValue, int8_t rightValue);
    void adcGain(int8_t leftGain, int8_t rightGain);
    void headphoneGain(int8_t leftGain, int8_t rightGain, bool mute);
    void lineOutGain(int8_t leftGain, int8_t rightGain, bool mute);
};
