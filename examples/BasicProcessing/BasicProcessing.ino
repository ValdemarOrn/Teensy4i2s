#include <Teensy_I2S.h>
#include <Wire.h>
#include <SPI.h>

AudioControlSGTL5000 audioShield;

int acc = 0;
void processAudio(int32_t** inputs, int32_t** outputs)
{
  for (size_t i = 0; i < AUDIO_BLOCK_SAMPLES; i++)
  {
    int sig = (int)(sinf(acc / 100.0f * 2.0f * M_PI) * 1000000000.0f);
    // sig = sig & 0xFFFFFF00; // 24 bit mask
    outputs[0][i] = inputs[0][i] + sig;
    outputs[1][i] = inputs[0][i] + sig;
    acc++;
    if (acc >= 100)
      acc -= 100;
  }
}

void setup(void)
{
  Serial.begin(9600);
  
  // Enable the audio CODEC and set the volume
  audioShield.enable();
  audioShield.volume(0.5);

  // Assign the callback function
  i2sAudioCallback = processAudio;

  // Start the I2S interrupts
  InitI2s();
}

void loop(void)
{
}