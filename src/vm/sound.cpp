#include "sound.h"

#include "memory.h"

#include <random>
#include <cassert>

using namespace retro8;
using namespace retro8::sfx;


inline void DSP::squareWave(uint32_t frequency, int16_t amplitude, int16_t offset, int32_t position, int16_t* dest, size_t samples)
{
  const size_t periodLength = float(rate) / frequency;
  const size_t halfPeriod = periodLength / 2;

  for (size_t i = 0; i < samples; ++i)
  {
    const auto sampleInPeriod = position % periodLength;
    dest[i] += offset + sampleInPeriod < halfPeriod ? -amplitude : amplitude;
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
    dest[i] += offset + sampleInPeriod < dutyOnLength ? amplitude : -amplitude;
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
      dest[i] += offset + amplitude - amplitude * 2 * (p / 0.5f);
    else
      dest[i] += offset - amplitude + amplitude * 2 * ((p - 0.5f) / 0.5f);

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
    dest[i] += offset - amplitude + amplitude * 2 * p;
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
      dest[i] += offset - amplitude + amplitude * 2 * (p / dutyCycle);
    else
    {
      const float op = (p - dutyCycle) / (1.0f - dutyCycle);
      dest[i] += offset + amplitude - amplitude * 2 * op;
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
      dest[i] += offset + amplitude - amplitude * 2 * (p / 0.25f);
    else if (p < 0.50f) // raise -a +c
      dest[i] += offset - amplitude + amplitude * (1.0f + coefficient) * (p - 0.25) / 0.25;
    else if (p < 0.75) // drop +c -a
      dest[i] += offset + amplitude * coefficient - amplitude * (1.0f + coefficient) * (p - 0.50) / 0.25f;
    else
      dest[i] += offset - amplitude + amplitude * 2 * (p - 0.75f) / 0.25f;

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
    dest[i] += dist(mt);
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

  for (auto& channel : channels) channel.sound = nullptr;
  
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

void APU::play(sound_index_t index, channel_index_t channel, uint32_t start, uint32_t end)
{
  queueMutex.lock();
  queue.push_back({ index, channel, start, end });
  queueMutex.unlock();
}

void APU::handleCommands()
{
  if (queueMutex.try_lock())
  {
    for (Command& c : queue)
    { 
      /* stop sound on channel*/
      if (c.index == -1)
      {
        assert(c.channel >= 0 && c.channel <= channels.size());
        channels[c.channel].sound = nullptr;
        continue;
      }
      /* stop sound from looping */
      else if (c.index == -2)
      {
        continue;
      }
      /* stop sound on all channels that are playing it*/
      else if (c.channel == -2)
      {
        for (auto& chan : channels)
          if (chan.soundIndex == c.index)
            chan.sound = nullptr;
        continue;
      }
      /* find first available channel*/
      else if (c.channel == -1)
        for (size_t i = 0; i < channels.size(); ++i)
          if (!channels[i].sound)
          {
            c.channel = i;
            break;
          }


      if (c.channel >= 0 && c.channel < channels.size() && c.index >= 0 && c.index <= SOUND_COUNT)
      {
        /* overtaking channel */
        auto& channel = channels[c.channel];

        channel.soundIndex = c.index;
        channel.sound = memory.sound(c.index);
        channel.end = c.end;
        channel.sample = c.start;

        size_t samplePerTick = (44100 / 128) * (channel.sound->speed + 1);

        channel.position = c.start*samplePerTick;
      }
    }

    queue.clear();
    queueMutex.unlock();
  }

  /* stop sound on channel*/
}

void APU::updateMusic()
{
  if (music.music)
  {
    for (channel_index_t i = 0; i < CHANNEL_COUNT; ++i)
    {
      /* will use channel if channel is forced or there's no sound currently playing there */
      bool willUseChannel = ((music.channelMask & (1 << i)) != 0) || !channels[i].sound;

      
    }

  }
  
}

void APU::renderSounds(int16_t* dest, size_t totalSamples)
{
  handleCommands();
  
  constexpr size_t rate = 44100;
  constexpr int16_t maxVolume = 4096;

  memset(dest, 0, sizeof(int16_t)*totalSamples);

  for (SoundState& state : channels)
  {
    if (state.sound)
    {
      int16_t* buffer = dest;
      size_t samples = totalSamples;
      size_t samplePerTick = (44100 / 128) * (state.sound->speed + 1);


      while (samples > 0 && state.sound)
      {
        /* generate the maximum amount of samples available for same note */
        // TODO: optimize if next note is equal to current
        size_t available = std::min(samples, samplePerTick - (state.position % samplePerTick));

        const SoundSample& sample = state.sound->samples[state.sample];

        const int16_t volume = (maxVolume / 8) * sample.volume();
        const frequency_t frequency = Note::frequency(sample.pitch());

        /* render samples */
        switch (sample.waveform())
        {
        case Waveform::SQUARE:
          dsp.squareWave(frequency, volume, 0, state.position, buffer, samples);
          break;
        case Waveform::TILTED_SAW:
          dsp.tiltedSawtoothWave(frequency, volume, 0, 0.85f, state.position, buffer, samples);
          break;
        case Waveform::SAW:
          dsp.sawtoothWave(frequency, volume, 0, state.position, buffer, samples);
          break;
        case Waveform::TRIANGLE:
          dsp.triangleWave(frequency, volume, 0, state.position, buffer, samples);
          break;
        case Waveform::PULSE:
          dsp.pulseWave(frequency, volume, 0, 1/3.0f, state.position, buffer, samples);
          break;
        case Waveform::ORGAN:
          dsp.organWave(frequency, volume, 0, 0.5f, state.position, buffer, samples);
          break;
        case Waveform::NOISE:
          dsp.noise(frequency, volume, state.position, buffer, samples);
          break;
        }
        

        samples -= available;
        buffer += available;
        state.position += available;
        state.sample = state.position / samplePerTick;

        if (state.sample >= state.end)
          state.sound = nullptr;
      }
    }
  }
}