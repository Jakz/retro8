#pragma once

#include "defines.h"
#include "common.h"

#include <SDL_audio.h>

#if SOUND_ENABLED

namespace retro8
{
  namespace sound
  {
    using volume_t = int32_t;
    using pitch_t = int32_t;
    
    enum class Waveform
    {
      TRIANGLE, TILTED_SAW, SAW, SQUARE, PULSE, ORGAN, NOISE, PHASER
    };

    enum class Effect
    {
      NONE, SLIDE, VIBRATO, DROP, FADE_IN, FADE_OUT, ARPEGGIO_FAST, ARPEGGIO_SLOW
    };

    struct SoundSample
    {
      //TODO: endianness is a fail here
      uint16_t value;

      bool useSfx() const { return value & 0x8000; }
      Effect effect() const { return Effect((value >> 12) & 0b111); }
      Waveform waveform() const { return Waveform((value >> 6) & 0b111); }
      volume_t volume() const { return (value >> 9) & 0b111; }
      pitch_t pitch() const { return value & 0x111111; }
    };

    struct Sound
    {
      std::array<SoundSample, 32> samples;
      uint8_t editorMode; // TODO ??
      uint8_t speed; // 1 note = 1/128 sec * speed
      uint8_t loopStart;
      uint8_t loopEnd;
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
    };
  }
}

#endif