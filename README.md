Teensy 4 I2S Library
====================

This is a no-nonsense I2S library for Teensy 4 and 4.1.
It was derived from the I2S code in Paul Stoffregen's [Teensy Audio Library](https://github.com/PaulStoffregen/Audio)
but has been stripped down to nothing but the I2S code and some basic support mechanisms.

## Features:

* 24 Bit, 48Khz audio processing
* Completely stand-alone library, does not depend on the Teensy Audio library.
* Retains some of the Codec controllers from the Audio Library, and adds new ones
