#include "daisysp.h"
#include "kxmx_bluemchen/src/kxmx_bluemchen.h"

using namespace kxmx;
using namespace daisy;
using namespace daisysp;

Bluemchen hw;

void UpdateControls() {}

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
    float samplerate;
    samplerate = hw.AudioSampleRate();

    hw.StartAdc();
    hw.StartAudio(AudioCallback);

    while (1)
    {
        UpdateControls();
    }
}