#include "pxt.h"

#include "dmac.h"
#include "SAMDDAC.h"
#include "Synthesizer.h"
#include "CodalConfig.h"

#define NOTE_PAUSE 20
class WSynthesizer {
  public:
    Synthesizer synth;
    CODAL_DAC dac;

    WSynthesizer()
        // DAC always on PA02 on SAMD21
        : dac(*lookupPin(PA02), synth.output) {
        synth.setSampleRate(dac.getSampleRate());
        synth.setVolume(64);
        synth.setTone(Synthesizer::SquareWaveTone);
        this->setAmp(true);
    }

    // turns on/off the speaker amp
    void setAmp(bool on) {
        // turn off speaker as needed
        auto pinAmp = LOOKUP_PIN(SPEAKER_AMP);
        if (pinAmp) {
            pinAmp->setDigitalValue(on ? 1 : 0);
        }
    }
};
SINGLETON(WSynthesizer);

enum class SoundOutputDestination {
    //% block="pin"
    Pin = 1,
    //% block="speaker"
    Speaker = 0,
};


// override analogWrite for PA02 to use DAC
namespace AnalogOutPinMethods {
void analogWrite(AnalogOutPin name, int value) {
    if (name->name == PA02) {
        auto pinAmp = LOOKUP_PIN(SPEAKER_AMP);
        if (pinAmp) pinAmp->setDigitalValue(0);
        getWSynthesizer()->dac.setValue(value);
    } else
        name->setAnalogValue(value);
}
}

namespace music {

Buffer tone; // empty buffer to hold custom tone

/**
* This function is deprecated.
*/
//% help=music/set-tone
//% weight=1 group="Tones"
//% deprecated blockHidden=1
//% blockId=music_set_tone block="set tone %buffer"
void setTone(Buffer buffer) {
    if (!buffer) return;

    if (buffer->length != TONE_WIDTH * sizeof(uint16_t))
        return; // invalid length

    if (!tone)
        registerGC((TValue*)&tone);
    decrRC(tone);
    tone = buffer; // keep a reference to the buffer
    incrRC(tone);

    auto synth = &getWSynthesizer()->synth;
    synth->setTone((const uint16_t*)tone->data);
}

/**
* Turn the on-board speaker on or off.
* @param out the destination for sounds generated by the synthesizer
*/
// weight=2
// blockId=music_set_output block="set output %out"
// parts="speaker" blockGap=8 advanced=true
void setOutput(SoundOutputDestination out) {
    getWSynthesizer()->setAmp(out == SoundOutputDestination::Speaker);
}

/**
* Set the output volume of the sound synthesizer.
* @param volume the volume 0...256, eg: 128
*/
//% blockId=synth_set_volume block="set volume %volume"
//% parts="speaker"
//% volume.min=0 volume.max=256
//% help=music/set-volume
//% weight=70
//% group="Volume"
void setVolume(int volume) {
    auto synth = &getWSynthesizer()->synth;
    synth->setVolume(max(0, min(1024, volume * 4)));
}

/**
* Play a tone through the speaker for some amount of time.
* @param frequency pitch of the tone to play in Hertz (Hz), eg: Note.C
* @param ms tone duration in milliseconds (ms), eg: BeatFraction.Half
*/
//% help=music/play-tone
//% blockId=music_play_note block="play tone|at %note=device_note|for %duration=device_beat"
//% parts="headphone" async
//% blockNamespace=music
//% weight=76 blockGap=8
//% group="Tone"
void playTone(int frequency, int ms) {
    auto synth = &getWSynthesizer()->synth;

    if (frequency <= 0) {
        synth->setFrequency(0, max(1, ms));
    } else {
        if (ms > 0) {
            int d = max(1, ms - NOTE_PAUSE); // allow for short rest
            int r = max(1, ms - d);
            synth->setFrequency((float) frequency, d);
            synth->setFrequency(0, r);
        } else {
            // ring
            synth->setFrequency((float) frequency);
        }
    }
    fiber_sleep(1);
}

}