#include "sound.h"

using namespace retro8;
using namespace retro8::sound;


inline void DSP::squareWave(uint32_t frequency, int16_t amplitude, int16_t offset, int32_t position, int16_t* dest, size_t samples)
{
  const size_t periodLength = float(rate) / frequency;
  const size_t halfPeriod = periodLength / 2;

  for (size_t i = 0; i < samples; ++i)
  {
    const auto sampleInPeriod = position % periodLength;
    dest[i] = offset + sampleInPeriod < halfPeriod ? -amplitude : amplitude;
    ++position;
  }
}

inline void DSP::sawtoothWave(uint32_t frequency, int16_t amplitude, int16_t offset, int32_t position, int16_t* dest, size_t samples)
{
  const size_t periodLength = float(rate) / frequency;

  for (size_t i = 0; i < samples; ++i)
  {
    const size_t repetitions = position / periodLength;
    const float p = position / float(periodLength) - repetitions;
    dest[i] = offset - amplitude + amplitude * 2 * p;
    ++position;
  }
}

inline void DSP::tiltedSawtoothWave(uint32_t frequency, int16_t amplitude, int16_t offset, float dutyCycle, int32_t position, int16_t* dest, size_t samples)
{
  const size_t periodLength = float(rate) / frequency;

  for (size_t i = 0; i < samples; ++i)
  {
    const size_t repetitions = position / periodLength;
    const float p = position / float(periodLength) - repetitions;

    if (p < dutyCycle)
      dest[i] = offset - amplitude + amplitude * 2 * (p / dutyCycle);
    else
    {
      const float op = (p - dutyCycle) / (1.0f - dutyCycle);
      dest[i] = offset + amplitude - amplitude * 2 * op;
    }

    ++position;
  }
}

static int pos = 0;
static float period = 44100 / 440.0f;

DSP dsp(44100);

void audio_callback(void* data, uint8_t* cbuffer, int length)
{
  APU* apu = static_cast<APU*>(data);
  int16_t* buffer = reinterpret_cast<int16_t*>(cbuffer);

  dsp.tiltedSawtoothWave(440, 4096, 0, 0.85f, pos, buffer, length / sizeof(int16_t));
  pos += length / sizeof(int16_t);
  return;

  for (int i = 0; i < length/sizeof(int16_t); ++i)
  {
    //SIN buffer[i] = std::sin(pos / period * M_PI * 2)*8192;
    int integral = (pos / period);
    float p = pos / period - integral;

    //TRIANGLE
    //float posInPeriod = pos / period - (int)(pos / period);
    //buffer[i] = (posInPeriod <= 0.5f ? (posInPeriod * 2) : (1.0 - ((posInPeriod - 0.5) * 2))) * 8192 + 4096;

    // SAWTOOTH
    buffer[i] = (p -0.5f)* 4096;

    // SAWTOOTH
    //float posInPeriod = pos / period - (int)(pos / period);
    //buffer[i] = (posInPeriod > 0.2 && posInPeriod < 0.8 ? (posInPeriod/0.6) : (1.0 - ((posInPeriod - 0.5) * 2))) * 4096;

    //SQUARE
    buffer[i] = p < 0.5f ? -4096 : 4096;

    //PULSE: dirtier!
    buffer[i] = p < 0.33f ? 4096 : -4096;

    //ORGAN
    bool odd = integral & 0x01;
    buffer[i] = (p <= 0.5f ? (p * 2) : (1.0 - ((p - 0.5) * 2))) * 8192 + 4096;


    ++pos;
  }
}

void APU::init()
{
  static_assert(sizeof(SoundSample) == 2, "Must be 2 bytes");
  static_assert(sizeof(Sound) == 68, "Must be 68 bytes");
  
  SDL_AudioSpec wantSpec;
  wantSpec.freq = 44100;
  wantSpec.format = AUDIO_S16SYS;
  wantSpec.channels = 1;
  wantSpec.samples = 2048;
  wantSpec.userdata = this;
  wantSpec.callback = audio_callback;
  
  device = SDL_OpenAudioDevice(NULL, 0, &wantSpec, &spec, 0);

  if (!device)
  {
    printf("Error while opening audio: %s", SDL_GetError());
  }
}

void APU::resume()
{
  SDL_PauseAudioDevice(device, false);
}

void APU::close()
{
  SDL_CloseAudioDevice(device);
}