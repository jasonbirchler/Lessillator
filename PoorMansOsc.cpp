#include "daisysp.h"
#include "kxmx_bluemchen/src/kxmx_bluemchen.h"

using namespace kxmx;
using namespace daisy;
using namespace daisysp;

Bluemchen hw;

int enc_val = 0;
int midi_note = 0;

Parameter knob1;
Parameter knob2;
Parameter cv1;
Parameter cv2;

void UpdateOled()
{
    FontDef font = Font_6x8;
    uint8_t charWidth = font.FontWidth;
    uint8_t lineHeight = font.FontHeight;
    uint8_t xOrigin = 0;
    uint8_t yOrigin = 0;

    hw.display.Fill(false);
    hw.display.SetCursor(xOrigin, yOrigin);

    // Display Encoder test increment value and pressed state
    std::string str = "Enc:";
    char *cstr = &str[0];
    hw.display.WriteString(cstr, font, true);

    str = std::to_string(enc_val);
    hw.display.SetCursor(5 * charWidth, yOrigin);
    hw.display.WriteString(cstr, font, !hw.encoder.Pressed());

    // Display the knob values in millivolts
    str = "K1:";
    hw.display.SetCursor(xOrigin, 1 * lineHeight);
    hw.display.WriteString(cstr, font, true);

    str = std::to_string(static_cast<int>(knob1.Value()));
    hw.display.SetCursor(4 * charWidth, 1 * lineHeight);
    hw.display.WriteString(cstr, font, true);

    str = "K2:";
    hw.display.SetCursor(xOrigin, 2 * lineHeight);
    hw.display.WriteString(cstr, font, true);

    str = std::to_string(static_cast<int>(knob2.Value()));
    hw.display.SetCursor(4 * charWidth, 2 * lineHeight);
    hw.display.WriteString(cstr, font, true);

    // Display MIDI input note number
    str = "M:";
    hw.display.SetCursor(xOrigin, 3 * lineHeight);
    hw.display.WriteString(cstr, font, true);

    str = std::to_string(static_cast<int>(midi_note));
    hw.display.SetCursor(2 * charWidth, 3 * lineHeight);
    hw.display.WriteString(cstr, font, true);

    hw.display.Update();
}

// Typical Switch case for Message Type.
void HandleMidiMessage(MidiEvent m)
{
    switch (m.type)
    {
    case NoteOn:
    {
        midi_note = m.AsNoteOn().note;
    }
    break;
    case NoteOff:
    {
        midi_note = 0;
    }
    break;
    default:
        break;
    }
}

void UpdateControls()
{
    hw.ProcessAllControls();

    knob1.Process();
    knob2.Process();

    cv1.Process();
    cv2.Process();

    enc_val += hw.encoder.Increment();
}

void AudioCallback(AudioHandle::InputBuffer in, AudioHandle::OutputBuffer out, size_t size)
{
    for(size_t i = 0; i < size; i++)
    {
        out[0][i] = in[0][i];
        out[1][i] = in[1][i];
    }
}

int main(void)
{
    hw.Init();
    hw.StartAdc();

    knob1.Init(hw.controls[hw.CTRL_1], 0.0f, 5000.0f, Parameter::LINEAR);
    knob2.Init(hw.controls[hw.CTRL_2], 0.0f, 5000.0f, Parameter::LINEAR);

    cv1.Init(hw.controls[hw.CTRL_3], -5000.0f, 5000.0f, Parameter::LINEAR);
    cv2.Init(hw.controls[hw.CTRL_4], -5000.0f, 5000.0f, Parameter::LINEAR);

    hw.StartAudio(AudioCallback);

    while (1)
    {
        UpdateControls();
        UpdateOled();

        hw.midi.Listen();
        while (hw.midi.HasEvents())
        {
            HandleMidiMessage(hw.midi.PopEvent());
        }
    }
}