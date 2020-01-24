#pragma once

#include "common.h"
#include "defines.h"

#include "vm/machine.h"

namespace retro8
{
  namespace input
  {
    class InputManager
    {
    private:
      Machine* _machine;
      uint32_t _frameCounter;

      struct KeyStatus
      {
        enum class State { OFF, FIRST, WAITING, REPEATING } state;
        retro8::button_t button;
        uint32_t ticks;
      };
      std::array<std::array<KeyStatus, retro8::BUTTON_COUNT>, retro8::PLAYER_COUNT> keyStatus;

    public:
      InputManager();

      void manageKeyRepeat();
      void manageKey(size_t playerIndex, size_t buttonIndex, bool pressed);
      void reset();
      void tick() { ++_frameCounter; }
      void setMachine(Machine* machine) { _machine = machine; }
    };

    inline InputManager::InputManager() : _frameCounter(0)
    {
      reset();
    }

    inline void InputManager::reset()
    {
      for (auto& pks : keyStatus)
      {
        for (uint32_t i = 0; i < pks.size(); ++i)
        {
          pks[i].button = retro8::button_t(1 << i);
          pks[i].state = KeyStatus::State::OFF;
        }
      }
    }

    inline void InputManager::manageKeyRepeat()
    {
      static constexpr uint32_t TICKS_FOR_FIRST_REPEAT = 15;
      static constexpr uint32_t TICKS_REPEATING = 4;

      /* manage key repeats */
      const uint32_t ticks = _frameCounter;

      for (size_t i = 0; i < keyStatus.size(); ++i)
      {
        auto& pks = keyStatus[i];

        for (KeyStatus& ks : pks)
        {
          if (ks.state == KeyStatus::State::FIRST)
          {
            _machine->state().previousButtons[i].set(ks.button);
            ks.state = KeyStatus::State::WAITING;
          }
          else if (ks.state == KeyStatus::State::WAITING && (ticks - ks.ticks) >= TICKS_FOR_FIRST_REPEAT)
          {
            _machine->state().previousButtons[i].set(ks.button);
            ks.state = KeyStatus::State::REPEATING;
            ks.ticks = ticks;
          }
          else if (ks.state == KeyStatus::State::REPEATING && (ticks - ks.ticks) >= TICKS_REPEATING)
          {
            _machine->state().previousButtons[i].set(ks.button);
            ks.ticks = ticks;
          }
          else
            _machine->state().previousButtons[i].reset(ks.button);
        }
      }
    }

    inline void InputManager::manageKey(size_t pindex, size_t index, bool pressed)
    {
      const auto bt = keyStatus[pindex][index].button;
      _machine->state().buttons[pindex].set(bt, pressed);
      keyStatus[pindex][index].ticks = _frameCounter;
      keyStatus[pindex][index].state = pressed ? KeyStatus::State::FIRST : KeyStatus::State::OFF;
    }
  }
}