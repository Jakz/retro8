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

inline void DSP::pulseWave(uint32_t frequency, int16_t amplitude, int16_t offset, float dutyCycle, int32_t position, int16_t* dest, size_t samples)
{
  const size_t periodLength = float(rate) / frequency;
  const size_t dutyOnLength = dutyCycle * periodLength;

  for (size_t i = 0; i < samples; ++i)
  {
    const auto sampleInPeriod = position % periodLength;
    dest[i] = offset + sampleInPeriod < dutyOnLength ? amplitude : -amplitude;
    ++position;
  }
}

inline void DSP::triangleWave(uint32_t frequency, int16_t amplitude, int16_t offset, int32_t position, int16_t* dest, size_t samples)
{
  const size_t periodLength = float(rate) / frequency;

  for (size_t i = 0; i < samples; ++i)
  {
    const size_t repetitions = position / periodLength;
    const float p = position / float(periodLength) - repetitions;

    if (p < 0.50f)
      dest[i] = offset + amplitude - amplitude * 2 * (p / 0.5f);
    else
      dest[i] = offset - amplitude + amplitude * 2 * ((p - 0.5f) / 0.5f);

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

inline void DSP::organWave(uint32_t frequency, int16_t amplitude, int16_t offset, float coefficient, int32_t position, int16_t* dest, size_t samples)
{
  const size_t periodLength = float(rate) / frequency;

  for (size_t i = 0; i < samples; ++i)
  {
    const size_t repetitions = position / periodLength;
    const float p = position / float(periodLength) - repetitions;

    if (p < 0.25f) // drop +a -a
      dest[i] = offset + amplitude - amplitude * 2 * (p / 0.25f);
    else if (p < 0.50f) // raise -a +c
      dest[i] = offset - amplitude + amplitude * (1.0f + coefficient) * (p - 0.25) / 0.25;
    else if (p < 0.75) // drop +c -a
      dest[i] = offset + amplitude * coefficient - amplitude * (1.0f + coefficient) * (p - 0.50) / 0.25f;
    else
      dest[i] = offset - amplitude + amplitude * 2 * (p - 0.75f) / 0.25f;

    ++position;
  }
}

static int pos = 0;
static float period = 44100 / 440.0f;

DSP dsp(44100);

// C C# D D# E F F# G G# A A# B

constexpr float PULSE_WAVE_DEFAULT_DUTY = 1 / 3.0f;
constexpr float ORGAN_DEFAULT_COEFFICIENT = 0.5f;

enum Note { C, CS, D, DS, E, F, FS, G, GS, A, AS, B };
const std::array<float, 12> NoteTable = {
  130.81f, 138.59f, 146.83f, 155.56f, 164.81f, 174.61f, 185.00f, 196.00, 207.65, 220.0, 233.08, 246.94
};

void audio_callback(void* data, uint8_t* cbuffer, int length)
{
  APU* apu = static_cast<APU*>(data);
  int16_t* buffer = reinterpret_cast<int16_t*>(cbuffer);

  dsp.triangleWave(NoteTable[A]*2, 4096, 0, pos, buffer, length / sizeof(int16_t));
  pos += length / sizeof(int16_t);
  return;
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