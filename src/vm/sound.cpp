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

constexpr std::array<float, 12> Note::frequencies;

constexpr float PULSE_WAVE_DEFAULT_DUTY = 1 / 3.0f;
constexpr float ORGAN_DEFAULT_COEFFICIENT = 0.5f;

size_t position = 0;
int16_t* rendered = nullptr;



void APU::init()
{
  static_assert(sizeof(SoundSample) == 2, "Must be 2 bytes");
  static_assert(sizeof(Sound) == 68, "Must be 68 bytes");
  for (auto& channel : channels) channel.sound = nullptr;
}

void APU::play(sound_index_t index, channel_index_t channel, uint32_t start, uint32_t end)
{
  queueMutex.lock();
  queue.emplace_back(index, channel, start, end);
  queueMutex.unlock();
}

void APU::music(music_index_t index, int32_t fadeMs, int32_t mask)
{
  queueMutex.lock();
  queue.emplace_back(index, fadeMs, mask);
  queueMutex.unlock();
}

void APU::handleCommands()
{
  if (queueMutex.try_lock())
  {
    for (Command& c : queue)
    {
      if (!c.isMusic)
      {
        auto& s = c.sound;

        /* stop sound on channel*/
        if (s.index == -1)
        {
          if (s.channel >= 0 && s.channel <= channels.size())
            channels[s.channel].sound = nullptr;
          continue;
        }
        /* stop sound from looping */
        else if (s.index == -2)
        {
          continue;
        }
        /* stop sound on all channels that are playing it*/
        else if (s.channel == -2)
        {
          for (auto& chan : channels)
            if (chan.soundIndex == s.index)
              chan.sound = nullptr;
          continue;
        }
        /* find first available channel*/
        else if (s.channel == -1)
          for (size_t i = 0; i < channels.size(); ++i)
            if (!channels[i].sound)
            {
              s.channel = i;
              break;
            }


        if (s.channel >= 0 && s.channel < channels.size() && s.index >= 0 && s.index <= SOUND_COUNT)
        {
          /* overtaking channel */
          auto& channel = channels[s.channel];

          channel.soundIndex = s.index;
          channel.sound = memory.sound(s.index);
          channel.end = s.end;
          channel.sample = s.start;

          size_t samplePerTick = (44100 / 128) * (channel.sound->speed + 1);

          channel.position = s.start*samplePerTick;
        }
      }
      else
      {
        const auto& m = c.music;

        if (m.index == -1)
          mstate.music = nullptr;
        else
        {
          mstate.pattern = m.index;
          mstate.music = memory.music(m.index);
          mstate.channelMask = m.mask;

          for (size_t i = 0; i < CHANNEL_COUNT; ++i)
          {
            if (mstate.music->isChannelEnabled(i))
            {
              mstate.channels[i].sound = memory.sound(mstate.music->sound(i));
              mstate.channels[i].sample = 0;
              mstate.channels[i].position = 0;
              mstate.channels[i].end = 31; //TODO: fix according to behavior
            }
            else
              mstate.channels[i].sound = nullptr;
          }
        }
      }
    }

    queue.clear();
    queueMutex.unlock();
  }

  /* stop sound on channel*/
}

void APU::updateMusic()
{
  if (mstate.music)
  {
    for (channel_index_t i = 0; i < CHANNEL_COUNT; ++i)
    {
      /* will use channel if channel is forced or there's no sound currently playing there */
      bool willUseChannel = ((mstate.channelMask & (1 << i)) != 0) || !channels[i].sound;


    }

  }

}

void APU::updateChannel(SoundState& channel, const Music* music)
{
  if (!music)
  {
    if (channel.sample >= channel.end)
      channel.sound = nullptr;
  }
  else
  {
    /* sound is ended, behavior depends on flag for music*/
    if (channel.sample >= channel.end)
    {
      if (music->isStop())
      {
        this->mstate.music = nullptr;
        for (auto& channel : mstate.channels)
          channel.sound = nullptr;
        return;
      }

      ++mstate.pattern;

      if (music->isLoopEnd() || mstate.pattern == MUSIC_COUNT)
      {
        music_index_t i = mstate.pattern - 1;
        const Music* next = nullptr;

        while (i >= 0)
        {
          next = memory.music(i);
          if (next->isLoopBegin() || i == 0)
            break;

          --i;
        }

        mstate.pattern = i;
        mstate.music = next;
      }

      mstate.music = memory.music(mstate.pattern);

      for (size_t i = 0; i < CHANNEL_COUNT; ++i)
      {
        if (mstate.music->isChannelEnabled(i))
        {
          mstate.channels[i].sound = memory.sound(mstate.music->sound(i));
          mstate.channels[i].sample = 0;
          mstate.channels[i].position = 0;
          mstate.channels[i].end = 31; //TODO: fix according to behavior
        }
        else
          mstate.channels[i].sound = nullptr;
      }
    }
  }
}

void APU::renderSound(const SoundState& channel, int16_t* buffer, size_t samples)
{
  const SoundSample& sample = channel.sound->samples[channel.sample];

  constexpr int16_t maxVolume = 4096;
  const int16_t volume = (maxVolume / 8) * sample.volume();
  const frequency_t frequency = Note::frequency(sample.pitch());

  /* render samples */
  switch (sample.waveform())
  {
  case Waveform::SQUARE:
    dsp.squareWave(frequency, volume, 0, channel.position, buffer, samples);
    break;
  case Waveform::TILTED_SAW:
    dsp.tiltedSawtoothWave(frequency, volume, 0, 0.85f, channel.position, buffer, samples);
    break;
  case Waveform::SAW:
    dsp.sawtoothWave(frequency, volume, 0, channel.position, buffer, samples);
    break;
  case Waveform::TRIANGLE:
    dsp.triangleWave(frequency, volume, 0, channel.position, buffer, samples);
    break;
  case Waveform::PULSE:
    dsp.pulseWave(frequency, volume, 0, 1 / 3.0f, channel.position, buffer, samples);
    break;
  case Waveform::ORGAN:
    dsp.organWave(frequency, volume, 0, 0.5f, channel.position, buffer, samples);
    break;
  case Waveform::NOISE:
    dsp.noise(frequency, volume, channel.position, buffer, samples);
    break;
  }
}

void APU::renderSounds(int16_t* dest, size_t totalSamples)
{
  handleCommands();

  constexpr size_t rate = 44100;
  constexpr int16_t maxVolume = 4096;

  memset(dest, 0, sizeof(int16_t)*totalSamples);

  for (size_t i = 0; i < CHANNEL_COUNT; ++i)
  {
    int16_t* buffer = dest;
    size_t samples = totalSamples;

    SoundState& channel = channels[i].sound ? channels[i] : mstate.channels[i];
    const Music* music = &channel == &this->mstate.channels[i] ? this->mstate.music : nullptr; //TODO: crappy comparison
  
    /* render only if enabled */
    if ((music && _musicEnabled) || (!music && _soundEnabled))
    {
      if (channel.sound)
      {
        const size_t samplePerTick = (44100 / 128) * (channel.sound->speed + 1);
        while (samples > 0 && channel.sound)
        {
          /* generate the maximum amount of samples available for same note */
          // TODO: optimize if next note is equal to current
          size_t available = std::min(samples, samplePerTick - (channel.position % samplePerTick));
          renderSound(channel, buffer, available);

          samples -= available;
          buffer += available;
          channel.position += available;
          channel.sample = channel.position / samplePerTick;

          updateChannel(channel, music);
        }
      }
    }
  }
}
