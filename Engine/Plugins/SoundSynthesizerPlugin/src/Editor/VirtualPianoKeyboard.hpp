#pragma once

#include <functional>

namespace SoundStudio
{

class VirtualPianoKeyboard
{
public:
    VirtualPianoKeyboard() = default;

    std::function<void(int midiNote, float velocity)> OnNoteOn;
    std::function<void()> OnNoteOff;

    void Draw(float width, float height);
};

} // namespace SoundStudio
