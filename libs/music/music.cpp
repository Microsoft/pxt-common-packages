#include "pxt.h"

#include "dmac.h"
#include "SAMD21DAC.h"
#include "Synthesizer.h"
#include "DeviceConfig.h"

#ifndef HAS_SPEAKER
#define HAS_SPEAKER (PIN_SPEAKER != NC)
#endif

class WSynthesizer {
  public:
    Synthesizer synth;
    SAMD21DAC dac;

    WSynthesizer()
        : dac(*lookupPin(PIN_SPEAKER), pxt::getWDMAC()->dmac, synth.output) {
        synth.setSampleRate(dac.getSampleRate());
        synth.setTone(Synthesizer::SquareWaveTone);
        synth.setVolume(0);
    }
};
SINGLETON(WSynthesizer);

enum class SoundOutputDestination {
    //% block="speaker"
    Speaker = 0,
    //% block="pin"
    Pin = 1
};

namespace music {

int synthVolume = 400;
PwmPin pitchPin;

#if HAS_SPEAKER
SoundOutputDestination current = SoundOutputDestination::Speaker;
#else
SoundOutputDestination current = SoundOutputDestination::Pin;
#endif

/**
* Sets the output destination for sounds generated by the music package
* @param out the destination for sounds generated by the music package
*/
//% weight=2
//% blockId=music_set_output block="set output %out"
//% parts="speaker" blockGap=8 advanced=true
void setOutput(SoundOutputDestination out) {
    current = out;
}

/**
* Sets the pin to output sound to when the sound output is set to "pin"
* @param pin the pin to generate sound on
*/
//% weight=1
//% blockId=music_set_pitch_pin block="set pitch pin %pin"
//% parts="speaker" blockGap=8 advanced=true
void setPitchPin(PwmPin pin) {
    pitchPin = pin;
}

/**
* Sets the output volume of the on board speaker (if available)
* @param volume the volume 0...256, eg: 128
*/
//% weight=96
//% blockId=synth_set_volume block="set speaker volume %volume"
//% parts="speaker" blockGap=8
//% volume.min=0 volume.max=256
//% weight=1
void setSpeakerVolume(int volume) {
    synthVolume = max(0, min(1024, volume * 4));
}

/**
* Plays a tone through the pin for the given duration.
* @param frequency pitch of the tone to play in Hertz (Hz)
* @param ms tone duration in milliseconds (ms)
*/
//% help=music/play-tone weight=90
//% blockId=music_play_note block="play tone|at %note=device_note|for %duration=device_beat"
//% parts="headphone" async
//% blockNamespace=music
void playTone(int frequency, int ms) {
    if (current == SoundOutputDestination::Speaker) {
        auto synth = &getWSynthesizer()->synth;
        
        if (frequency <= 0) {
            if (ms > 0) {
                synth->setFrequency(0, max(1, ms - 5));
            } else {
                synth->setVolume(0);
            }
            fiber_sleep(max(1, ms));
        } else {
            synth->setVolume(synthVolume);

            if (ms > 0) {
                synth->setFrequency((float) frequency, max(1, ms - 5));
                fiber_sleep(ms);
            } else {
                synth->setFrequency((float) frequency);                
                fiber_sleep(1);
            }
        }
    }
    else {
        if (NULL == pitchPin)
            pitchPin = getPin(PIN_A0);        
        if (frequency <= 0) {
            pitchPin->setAnalogValue(0);
        } else {
            pitchPin->setAnalogValue(512);
            pitchPin->setAnalogPeriodUs(1000000/frequency);
        }

        if (ms > 0) {
            fiber_sleep(max(1, ms - 5));
            pitchPin->setAnalogValue(0);
            wait_ms(5);
        }
        fiber_sleep(1);
    }
}
}