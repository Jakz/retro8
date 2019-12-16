#pragma once

#include "defines.h"
#include "common.h"

#include <array>
#include <SDL_audio.h>

#if SOUND_ENABLED

namespace retro8
{
  namespace sfx
  {
    using volume_t = int32_t;
    using pitch_t = int32_t;
    using frequency_t = int32_t;
    
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
        130.81f, 138.59f, 146.83f, 155.56f, 164.81f, 174.61f, 185.00f, 196.00, 207.65, 220.0, 233.08, 246.94
      };

    public:
      static pitch_t pitch(Tone tone, int32_t octave = 1) { return pitch_t(tone) + octave * 12; }
      static frequency_t frequency(Tone tone, int32_t octave = 1) { return frequencies[size_t(tone)] * octave; };
      static frequency_t frequency(pitch_t pitch) { return frequencies[pitch % 12] * (1 + pitch / 12); }
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

      bool useSfx() const { return value & 0x8000; }
      Effect effect() const { return Effect((value & EffectMask) >> EffectShift); }
      Waveform waveform() const { return Waveform((value & WaveformMask) >> WaveformShift); }
      volume_t volume() const { return (value & VolumeMask) >> VolumeShift; }
      pitch_t pitch() const { return value & PitchMask; }
      
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
    };

    struct Music
    {
      uint8_t dummy[4];
    };

    using sound_t = Sound;
    using music_t = Music;

    static constexpr size_t SOUND_COUNT = 64;
    static constexpr size_t MUSIC_COUNT = 64;

    struct SoundState
    {
      const Sound* sound;
      uint32_t sample;
      uint32_t position; // absolute
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
      SDL_AudioSpec spec;
      SDL_AudioDeviceID device;
    
    public:
      void init();
      void close();

      void resume();
      void pause();

      void renderSounds(int16_t* dest, size_t samples);
    };
  }
}

#endif