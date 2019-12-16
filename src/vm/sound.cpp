#include "sound.h"

#include <random>

using namespace retro8;
using namespace retro8::sfx;


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

inline void DSP::noise(uint32_t frequency, int16_t amplitude, int32_t position, int16_t* dest, size_t samples)
{
  static std::random_device rdevice;
  static std::mt19937 mt(rdevice());

  std::uniform_int_distribution<int16_t> dist(-amplitude/2, amplitude/2);

  const size_t periodLength = float(rate) / frequency;
  const size_t halfPeriod = periodLength / 2;

  for (size_t i = 0; i < samples; ++i)
  {
    const auto sampleInPeriod = position % periodLength;
    dest[i] = dist(mt);
    ++position;
  }
}

void DSP::fadeIn(int16_t amplitude, int16_t* dest, size_t samples)
{
  const float incr = 1.0f / samples;

  for (size_t i = 0; i < samples; ++i)
  {
    const float v = dest[i] / amplitude;
    dest[i] = v * incr * i * amplitude;
  }
}

void DSP::fadeOut(int16_t amplitude, int16_t* dest, size_t samples)
{
  const float incr = 1.0f / samples;

  for (size_t i = 0; i < samples; ++i)
  {
    const float v = dest[i] / amplitude;
    dest[i] = v * incr * (samples - i - 1) * amplitude;
  }
}

static int pos = 0;
static float period = 44100 / 440.0f;

DSP dsp(44100);

// C C# D D# E F F# G G# A A# B

constexpr float PULSE_WAVE_DEFAULT_DUTY = 1 / 3.0f;
constexpr float ORGAN_DEFAULT_COEFFICIENT = 0.5f;

size_t position = 0;
int16_t* rendered = nullptr;

void audio_callback(void* data, uint8_t* cbuffer, int length)
{
  APU* apu = static_cast<APU*>(data);
  int16_t* buffer = reinterpret_cast<int16_t*>(cbuffer);
  apu->renderSounds(buffer, length / sizeof(int16_t));
  return;
  
  if (!rendered)
  {
    rendered = new int16_t[44100 * 3];
    dsp.squareWave(440, 4096, 0, 0, rendered, 44100 * 3);
    dsp.fadeIn(4096, rendered, 44100);
    dsp.fadeOut(4096, rendered + 88200, 44100);

  }
  
  //
  //int16_t* buffer = reinterpret_cast<int16_t*>(cbuffer);

  if (position < 44100 * 3)
  {
    size_t max = std::min((44100 * 3 - position) * 2, size_t(length));
    memcpy(cbuffer, rendered + position, max);
    position += max / sizeof(int16_t);
  }
  else
    memset(cbuffer, 0, length);

  //dsp.triangleWave(NoteTable[A]*2, 4096, 0, pos, buffer, length / sizeof(int16_t));
  //dsp.noise(NoteTable[A] * 2, 4096, pos, buffer, length / sizeof(int16_t));
  //pos += length / sizeof(int16_t);
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

Sound ssound;
SoundState sstate;

void APU::resume()
{
  ssound.speed = 32;
  ssound.samples[0].setPitch(Note::pitch(Tone::E, 2));
  ssound.samples[1].setPitch(Note::pitch(Tone::E, 2));
  ssound.samples[2].setPitch(Note::pitch(Tone::F, 2));
  ssound.samples[3].setPitch(Note::pitch(Tone::G, 2));
  ssound.samples[4].setPitch(Note::pitch(Tone::G, 2));
  ssound.samples[5].setPitch(Note::pitch(Tone::F, 2));
  ssound.samples[6].setPitch(Note::pitch(Tone::E, 2));
  ssound.samples[7].setPitch(Note::pitch(Tone::D, 2));
  ssound.samples[8].setPitch(Note::pitch(Tone::C, 2));
  ssound.samples[9].setPitch(Note::pitch(Tone::C, 2));
  ssound.samples[10].setPitch(Note::pitch(Tone::D, 2));
  ssound.samples[11].setPitch(Note::pitch(Tone::E, 2));
  ssound.samples[12].setPitch(Note::pitch(Tone::E, 2));
  ssound.samples[13].setPitch(Note::pitch(Tone::D, 2));


  for (size_t i = 0; i < ssound.samples.size(); ++i)
  {
    ssound.samples[i].setVolume(7);
    ssound.samples[i].setWaveform(Waveform::ORGAN);
    printf("%d ", Note::frequency(ssound.samples[i].pitch()));
  }

  sstate.sound = &ssound;
  sstate.position = 0;
  sstate.sample = 0;
  
  SDL_PauseAudioDevice(device, false);
}

void APU::close()
{
  SDL_CloseAudioDevice(device);
}


void APU::renderSounds(int16_t* dest, size_t samples)
{
  constexpr size_t rate = 44100;
  size_t samplePerTick = (44100 / 128) * (sstate.sound->speed + 1);

  while (samples > 0)
  {
    /* generate the maximum amount of samples available for same note */
    // TODO: optimize if next note is equal to current
    size_t available = std::min(samples, samplePerTick - (sstate.position % samplePerTick));
    const SoundSample& sample = sstate.sound->samples[sstate.sample];

    /* render samples */
    dsp.squareWave(Note::frequency(sample.pitch()), (4096 / 8) * sample.volume(), 0, sstate.position, dest, samples);

    samples -= available;
    dest += available;
    sstate.position += available;
    sstate.sample = sstate.position / samplePerTick;
  }



}