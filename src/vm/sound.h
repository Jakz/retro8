#pragma once

#include "defines.h"
#include "common.h"

#include <array>
#include <vector>
#include <mutex>

#if SOUND_ENABLED

namespace retro8
{
  class Memory;
  
  namespace sfx
  {
    using volume_t = int32_t;
    using pitch_t = int32_t;
    using frequency_t = int32_t;
    using channel_index_t = int32_t;
    using sound_index_t = int32_t;
    using music_index_t = int32_t;
    
    enum class Waveform
    {
      TRIANGLE, TILTED_SAW, SAW, SQUARE, PULSE, ORGAN, NOISE, PHASER
    };

    enum class Effect
    {
      NONE, SLIDE, VIBRATO, DROP, FADE_IN, FADE_OUT, ARPEGGIO_FAST, ARPEGGIO_SLOW
    };

    enum class Tone { C, CS, D, DS, E, F, FS, G, GS, A, AS, B };
    struct Note
    {
    private:
      constexpr static std::array<float, 12> frequencies = {
        //16.35, 17.32f, 18.35f, 19.45f, 20.60f, 21.83f, 23.12f, 24.50f, 25.96f, 27.50f, 29.14f, 30.87f
        130.81f, 138.59f, 146.83f, 155.56f, 164.81f, 174.61f, 185.00f, 196.00, 207.65, 220.0, 233.08, 246.94
      };

    public:
      static pitch_t pitch(Tone tone, int32_t octave = 1) { return pitch_t(tone) + octave * 12; }
      static frequency_t frequency(Tone tone, int32_t octave = 1) { return frequencies[size_t(tone)] * octave; };
        static frequency_t frequency(pitch_t pitch) { return frequencies[pitch % 12] / 2 * (1 << (pitch / 12)); }
    };

    struct SoundSample
    {
      static constexpr uint16_t EffectMask = 0x7000;
      static constexpr uint16_t VolumeMask = 0x0E00;
      static constexpr uint16_t WaveformMask = 0x01C0;
      static constexpr uint16_t PitchMask = 0x003F;

      static constexpr uint32_t EffectShift = 12;
      static constexpr uint32_t VolumeShift = 9;
      static constexpr uint32_t WaveformShift = 6;
      
      //TODO: endianness is a fail here
      uint16_t value;

      inline bool useSfx() const { return value & 0x8000; }
      inline Effect effect() const { return Effect((value & EffectMask) >> EffectShift); }
      inline Waveform waveform() const { return Waveform((value & WaveformMask) >> WaveformShift); }
      inline volume_t volume() const { return (value & VolumeMask) >> VolumeShift; }
      inline pitch_t pitch() const { return value & PitchMask; }
      
      void setPitch(pitch_t pitch) { value = (value & ~PitchMask) | pitch; }
      void setVolume(volume_t volume) { value = (value & ~VolumeMask) | (volume << VolumeShift); }
      void setEffect(Effect effect) { value = (value & ~EffectMask) | (uint16_t(effect) << EffectShift); }
      void setWaveform(Waveform waveform) { value = (value & ~WaveformMask) | (uint16_t(waveform) << WaveformShift); }
    };

    struct Sound
    {
      std::array<SoundSample, 32> samples;
      uint8_t editorMode; // TODO ??
      uint8_t speed; // 1 note = 1/128 sec * speed
      uint8_t loopStart;
      uint8_t loopEnd;

      int32_t length() const
      {        
        for (int32_t l = samples.size() - 1; l > 0; --l)
        {
          if (samples[l].volume() > 0)
            return l;
        }

        return 1;
      }
    };

    struct Music
    {
    private:
      constexpr static uint8_t SOUND_INDEX_MASK = 0b00111111;
      constexpr static uint8_t CONFIG_MASK = 0b11000000;
      constexpr static uint8_t SOUND_ON_FLAG = 0b01000000;
      constexpr static uint8_t LOOP_FLAG = 0b10000000;

      std::array<uint8_t, 4> indices;

    public:
      
      void setSound(channel_index_t channel, sound_index_t index) { indices[channel] = (indices[channel] & CONFIG_MASK) | index | SOUND_ON_FLAG; }
      void markLoopBegin() { indices[0] |= LOOP_FLAG; }
      void markLoopEnd() { indices[1] |= LOOP_FLAG; }
      void markStop() { indices[2] |= LOOP_FLAG; }

      inline bool isLoopBegin() const { return (indices[0] & LOOP_FLAG) != 0; }
      inline bool isLoopEnd() const { return (indices[1] & LOOP_FLAG) != 0; }
      inline bool isStop() const { return (indices[2] & LOOP_FLAG) != 0; }

      inline bool isChannelEnabled(channel_index_t channel) const { return (indices[channel] & SOUND_ON_FLAG) != 0; }
      sound_index_t sound(channel_index_t channel) const { return indices[channel] & SOUND_INDEX_MASK; }
    };

    using sound_t = Sound;
    using music_t = Music;

    static constexpr size_t SOUND_COUNT = 64;
    static constexpr size_t MUSIC_COUNT = 64;
    static constexpr size_t TICKS_PER_SECOND = 128;

    struct SoundState
    {
      const Sound* sound;
      uint32_t soundIndex;
      uint32_t sample;
      uint32_t position; // absolute
      uint32_t end;
    };

    struct MusicState
    {
      std::array<SoundState, 4> channels;
      const Music* music;
      music_index_t pattern;
      uint8_t channelMask;
    };
    
    class DSP
    {
    private:
      int32_t rate;

    public:
      DSP(int32_t rate) : rate(rate) { }
      void squareWave(uint32_t frequency, int16_t amplitude, int16_t offset, int32_t position, int16_t* dest, size_t samples);
      void pulseWave(uint32_t frequency, int16_t amplitude, int16_t offset, float dutyCycle, int32_t position, int16_t* dest, size_t samples);
      void triangleWave(uint32_t frequency, int16_t amplitude, int16_t offset, int32_t position, int16_t* dest, size_t samples);
      void sawtoothWave(uint32_t frequency, int16_t amplitude, int16_t offset, int32_t position, int16_t* dest, size_t samples);
      void tiltedSawtoothWave(uint32_t frequency, int16_t amplitude, int16_t offset, float dutyCycle, int32_t position, int16_t* dest, size_t samples);
      void organWave(uint32_t frequency, int16_t amplitude, int16_t offset, float coefficient, int32_t position, int16_t* dest, size_t samples);
      void noise(uint32_t frequency, int16_t amplitude, int32_t position, int16_t* dest, size_t samples);

      void fadeIn(int16_t amplitude, int16_t* dest, size_t samples);
      void fadeOut(int16_t amplitude, int16_t* dest, size_t samples);

    };


    class APU
    {
    public:
      static constexpr size_t CHANNEL_COUNT = 4;

    private:
      retro8::Memory& memory;
      
      struct Command
      {
        bool isMusic;

        union
        {
          struct
          {
            sound_index_t index;
            channel_index_t channel;
            uint32_t start;
            uint32_t end;
          } sound;

          struct
          {
            music_index_t index;
            int32_t fadeMs;
            int32_t mask;
          } music;
        };

        Command(sound_index_t index, channel_index_t channel, uint32_t start, uint32_t end) : isMusic(false), sound({ index, channel, start, end }) { }
        Command(music_index_t index, int32_t fadeMs, int32_t mask) : isMusic(true), music({ index, fadeMs, mask }) { }
      };

      std::array<SoundState, CHANNEL_COUNT> channels;
      MusicState mstate;

      std::mutex queueMutex;
      std::vector<Command> queue;

      bool _soundEnabled, _musicEnabled;

      void handleCommands();

      void updateMusic();
      void renderSound(const SoundState& sound, int16_t* buffer, size_t samples);
      void updateChannel(SoundState& channel, const Music* music);

      

    public:
      APU(Memory& memory) : memory(memory), _soundEnabled(true), _musicEnabled(true) { }

      void init();

      void play(sound_index_t index, channel_index_t channel, uint32_t start, uint32_t end);
      void music(music_index_t index, int32_t fadeMs, int32_t mask);

      void renderSounds(int16_t* dest, size_t samples);

      bool isMusicEnabled() const { return _musicEnabled; }
      bool isSoundEnabled() const { return _soundEnabled; }

      void toggleSound(bool active) { _soundEnabled = active; }
      void toggleMusic(bool active) { _musicEnabled = active; }
    };
  }
}

#endif