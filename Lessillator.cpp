#include "daisysp.h"
#include "kxmx_bluemchen/src/kxmx_bluemchen.h"

using namespace kxmx;
using namespace daisy;
using namespace daisysp;

Bluemchen  hw;
Oscillator osc[4];
Svf        filt;

int enc_val = 0;
int midi_note = 0;
bool midi_note_rcd = false;
bool cc_msg_rcd = false;

Parameter knob1;
Parameter knob2;
Parameter cv1;
Parameter cv2;

uint selectedOsc = 0;
int cursorPosition = 0;
bool editState = false;

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
    // hw.display.WriteString(cstr, font, true);

    // str = std::to_string(enc_val);
    // hw.display.SetCursor(5 * charWidth, yOrigin);
    // hw.display.WriteString(cstr, font, !hw.encoder.Pressed());

    // Display the knob values in millivolts
    // str = "K1:";
    // hw.display.SetCursor(xOrigin, 1 * lineHeight);
    // hw.display.WriteString(cstr, font, true);

    // str = std::to_string(static_cast<int>(knob1.Value()));
    // hw.display.SetCursor(4 * charWidth, 1 * lineHeight);
    // hw.display.WriteString(cstr, font, true);

    // str = "K2:";
    // hw.display.SetCursor(xOrigin, 2 * lineHeight);
    // hw.display.WriteString(cstr, font, true);

    // str = std::to_string(static_cast<int>(knob2.Value()));
    // hw.display.SetCursor(4 * charWidth, 2 * lineHeight);
    // hw.display.WriteString(cstr, font, true);

    // Display MIDI input note number
    str = "M";
    hw.display.SetCursor(58, 2 * lineHeight);
    hw.display.WriteString(cstr, font, !midi_note_rcd);

    // Display MIDI CC number
    str = "CC";
    hw.display.SetCursor(58, 3 * lineHeight);
    hw.display.WriteString(cstr, font, !cc_msg_rcd);

    switch (cursorPosition)
    {
    case 0:
        hw.display.DrawRect(0,0,28,16,true, editState);
        break;
    case 1:
        hw.display.DrawRect(28,0,56,16,true, editState);
        break;
    case 2:
        hw.display.DrawRect(0,16,28,31,true, editState);
        break;
    case 3:
        hw.display.DrawRect(28,16,56,31,true, editState);
        break;
    default:
        break;
    }
    hw.display.DrawLine(0,16,56,16,true);
    hw.display.DrawLine(28,0,28,32,true);

    hw.display.Update();
}

// Typical Switch case for Message Type.
void HandleMidiMessage(MidiEvent m)
{
    switch (m.type)
    {
    case NoteOn:
    {
        NoteOnEvent p = m.AsNoteOn();
        p = m.AsNoteOn();

        midi_note_rcd = true;
        midi_note = p.note;

        for (size_t i = 0; i < 4; i++)
        {
            osc[i].SetFreq(mtof(p.note));
            osc[i].SetAmp((p.velocity / 127.0f));
        }
    }
    break;
    case NoteOff:
    {
        midi_note_rcd = false;
        for (size_t i = 0; i < 4; i++)
        {
            osc[i].SetAmp(0.f);
        }
        
    }
    break;
    case ControlChange:
    {
        ControlChangeEvent p = m.AsControlChange();

        cc_msg_rcd = p.value > 0 ? true : false;
        switch(p.control_number)
        {
            case 1:
                // CC 1 for cutoff.
                filt.SetFreq(mtof((float)p.value));
                break;
            case 2:
                // CC 2 for res.
                filt.SetRes(((float)p.value / 127.0f));
                break;
            default: break;
        }
        break;
    }
    default:
        cc_msg_rcd = false;
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

    cursorPosition += hw.encoder.Increment();
    if (cursorPosition == 4)
    {
        cursorPosition = 3;
    }
    if (cursorPosition == -1)
    {
        cursorPosition = 0;
    }

    editState = hw.encoder.RisingEdge() ? !editState : editState;
}

void SplashScreen()
{
    std::string str  = "PoManOsc";
    FontDef font = Font_7x10;
    char*       cstr = &str[0];
    uint16_t maxDelay = 100;

    hw.display.SetCursor(0,0);

    for (size_t i = 0; i < 50; i++)
    {
        uint16_t x = i < hw.display.Width() ? i : 0;
        uint16_t y = i < hw.display.Height() ? i : 0;
        hw.display.SetCursor(x, y);
        hw.display.WriteString(cstr, font, true);
        hw.display.Update();
        hw.DelayMs(maxDelay - 10);
    }
}

void AudioCallback(AudioHandle::InputBuffer in, AudioHandle::OutputBuffer out, size_t size)
{
    UpdateControls();

    float sig;

    for(size_t i = 0; i < size; i++)
    {
        for (size_t i = 0; i < 4; i++)
        {
            sig += osc[i].Process();
        }
        
        filt.Process(sig);

        out[0][i] = filt.Low();
        out[1][i] = filt.Low();
    }
}

int main(void)
{
    hw.Init();
    float samplerate = hw.AudioSampleRate();

    for (size_t i = 0; i < 4; i++)
    {
        osc[i].Init(samplerate);
        osc[i].SetWaveform(0);
    }
    

    knob1.Init(hw.controls[hw.CTRL_1], 0.0f, 5000.0f, Parameter::LINEAR);
    knob2.Init(hw.controls[hw.CTRL_2], 0.0f, 5000.0f, Parameter::LINEAR);

    cv1.Init(hw.controls[hw.CTRL_3], -5000.0f, 5000.0f, Parameter::LINEAR);
    cv2.Init(hw.controls[hw.CTRL_4], -5000.0f, 5000.0f, Parameter::LINEAR);

    // splash screen on boot
    // SplashScreen();

    hw.StartAdc();
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