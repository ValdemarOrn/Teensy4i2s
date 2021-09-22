#include <Teensy_I2S.h>
#include <Wire.h>
#include <SPI.h>
#include <arm_math.h>

AudioControlSGTL5000 audioShield;

int acc = 0;
void processAudio(int32_t** inputs, int32_t** outputs)
{
  for (size_t i = 0; i < AUDIO_BLOCK_SAMPLES; i++)
  {
    // use can use regular sinf() as well, but it's highly recommended 
    // to use these optimised arm-specific functions whenever possible
    int sig = (int)(arm_sin_f32(acc * 0.01f * 2.0f * M_PI) * 1000000000.0f);
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

  // Assign the callback function
  i2sAudioCallback = processAudio;

  // Start the I2S interrupts
  InitI2s();

  // need to wait a bit before configuring codec, otherwise something weird happens and there's no output...
  delay(1000); 
  // Enable the audio CODEC and set the volume
  audioShield.enable();
  audioShield.volume(0.5);
}

void loop(void)
{
  delay(1000);
  float avg = Timers::GetPeak(Timers::TIMER_TOTAL);
  float period = Timers::GetAvgPeriod();
  float percent = avg / period * 100;
  Serial.print("CPU Usage: ");
  Serial.print(percent, 4);
  Serial.print("%");
  Serial.print(" -- Processing period: ");
  Serial.print(period/1000, 3);
  Serial.println("ms");
  
}
