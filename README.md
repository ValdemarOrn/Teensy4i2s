Teensy 4 I2S Library
====================

This is a no-nonsense I2S library for Teensy 4 and 4.1.
It was derived from the I2S code in Paul Stoffregen's [Teensy Audio Library](https://github.com/PaulStoffregen/Audio)
but has been stripped down to nothing but the I2S code and some basic support mechanisms.

## Features

* 16/24/32 Bit, 44.1/48/96 Khz audio processing
* Completely stand-alone library, does not depend on the Teensy Audio library.
* Retains some of the Codec controllers from the Audio Library
* Adds codec controller for TI TLV320AIC3204

## Pinout

Required pinout is the same as the Teensy Audio Board and the Teensy Audio library

    MCLK	23      Audio Master Clock - Speed: 256 * Fs
    BCLK	21      Audio Bit Clock (aka. Serial Clock / SCLK) - Speed: 64 * Fs
    LRCLK	20      Audio Left/Right Clock (aka. Word Clock / WCLK) - Speed: Fs
    DIN     7       Audio Data from Teensy to Codec
    DOUT	8       Audio Data from Codex to Teensy
    SCL	    19      I2C Control Clock
    SDA	    18      I2C Control Data

## Frequencies

At 44.1Khz:

    MCLK    11.289,600 Mhz
    BCLK    2.822,400 Mhz
    LRCLK   44.1 Khz

At 48Khz:

    MCLK    12.288 Mhz
    BCLK    3.072 Mhz
    LRCLK   48.0 Khz

## Notes

Please note that the library always transmits and receives 32 bits between the codec and Teensy. Please ensure you shift your input and output values appropriately in code to work at your desired bit depth.
For example, for 24 bit data, you must multiply output data by 256, or left-shift by 8 bits. Similarly, for input data, you must right shift by 8 bits or divide by 256 to obtain a 24 bit value.