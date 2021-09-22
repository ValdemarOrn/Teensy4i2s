#include <Teensy_I2S.h>
#include <Wire.h>
#include <SPI.h>

AudioControlSGTL5000 audioShield;

int acc = 0;
void processAudio(int16_t** inputs, int16_t** outputs)
{
  for (size_t i = 0; i < AUDIO_BLOCK_SAMPLES; i++)
  {
    int sig = (int)(sinf(acc / 200.0f * 2.0f * M_PI) * 10000.0f);
    outputs[0][i] = inputs[0][i] + sig;
    outputs[1][i] = inputs[1][i] + sig;
    acc++;
    if (acc >= 200)
      acc -= 200;
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
