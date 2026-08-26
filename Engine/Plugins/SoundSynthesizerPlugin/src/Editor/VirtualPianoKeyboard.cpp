#include "VirtualPianoKeyboard.hpp"
#include "Utils/TEString.hpp"
#include "Utils/TimeGUI.hpp"

namespace SoundStudio
{

static const char *s_NoteNames[12] = {"C", "C#", "D", "D#", "E", "F", "F#", "G", "G#", "A", "A#", "B"};
static const bool s_IsBlackKey[12] = {false, true, false, true, false, false, true, false, true, false, true, false};

void VirtualPianoKeyboard::Draw(float width, float height)
{
    TimeGUI::TextColored(TEVector4(0.8f, 0.85f, 0.95f, 1.0f), "Virtual Synthesizer Keyboard (C3 - B4):");

    int startMidi = 48; // C3
    int numKeys = 24;   // 2 octaves

    for (int i = 0; i < numKeys; ++i)
    {
        int note = startMidi + i;
        int noteInOctave = note % 12;
        int octave = (note / 12) - 1;
        bool isBlack = s_IsBlackKey[noteInOctave];

        TEString label = TEString(s_NoteNames[noteInOctave]) + TEString::FromInt(octave);

        if (isBlack)
        {
            TimeGUI::PushStyleColor(TimeGUICol_Button, TEVector4(0.15f, 0.15f, 0.18f, 1.0f));
            TimeGUI::PushStyleColor(TimeGUICol_Text, TEVector4(0.9f, 0.9f, 0.9f, 1.0f));
        }
        else
        {
            TimeGUI::PushStyleColor(TimeGUICol_Button, TEVector4(0.9f, 0.9f, 0.95f, 1.0f));
            TimeGUI::PushStyleColor(TimeGUICol_Text, TEVector4(0.1f, 0.1f, 0.1f, 1.0f));
        }

        if (TimeGUI::Button(label.c_str(), TEVector2(32.0f, height)))
        {
            if (OnNoteOn)
                OnNoteOn(note, 1.0f);
        }

        TimeGUI::PopStyleColor(2);
        TimeGUI::SameLine();
    }
    TimeGUI::NewLine();
}

} // namespace SoundStudio
