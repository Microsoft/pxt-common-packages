#include "pxt.h"

enum class PulseValue {
    //% block=high
    High = DEVICE_PIN_EVT_PULSE_HI,
    //% block=low
    Low = DEVICE_PIN_EVT_PULSE_LO
};

enum class PinEvent {
    //% block="pulse high"
    PulseHigh = DEVICE_PIN_EVT_PULSE_HI,
    //% block="pulse low"
    PulseLow = DEVICE_PIN_EVT_PULSE_LO,
    //% block="rise"
    Rise = DEVICE_PIN_EVT_RISE,
    //% block="fall"
    Fall = DEVICE_PIN_EVT_FALL,
};

enum class PinPullMode {
    //% block="down"
    PullDown = 0,
    //% block="up"
    PullUp = 1,
    //% block="none"
    PullNone = 2
};

namespace DigitalInOutPinMethods {
/**
 * Read a pin or connector as either 0 or 1
 * @param name pin to read from
 */
//% help=pins/digital-read weight=61
//% blockId=device_get_digital_pin block="digital read|pin %name" blockGap=8
//% blockNamespace=pins
//% name.fieldEditor="gridpicker"
//% name.fieldOptions.width=220
//% name.fieldOptions.columns=4
bool digitalRead(DigitalInOutPin name) {
    return PINOP(getDigitalValue()) != 0;
}

/**
     * Set a pin or connector value to either 0 or 1.
    * @param name pin to write to
    * @param value value to set on the pin
    */
//% help=pins/digital-write weight=60
//% blockId=device_set_digital_pin block="digital write|pin %name|to %value=toggleHighLow"
//% blockNamespace=pins
//% name.fieldEditor="gridpicker"
//% name.fieldOptions.width=220
//% name.fieldOptions.columns=4
void digitalWrite(DigitalInOutPin name, bool value) {
    PINOP(setDigitalValue(value));
}

/**
* Make this pin a digital input, and create events where the timestamp is the duration
* that this pin was either ``high`` or ``low``.
*/
//% help=pins/on-pulsed weight=16 blockGap=8
//% blockId=pins_on_pulsed block="on|pin %pin|pulsed %pulse"
//% blockNamespace=pins
//% pin.fieldEditor="gridpicker"
//% pin.fieldOptions.width=220
//% pin.fieldOptions.columns=4
//% deprecated=1 hidden=1
void onPulsed(DigitalInOutPin pin, PulseValue pulse, Action body) {
    pin->eventOn(DEVICE_PIN_EVENT_ON_PULSE);
    registerWithDal(pin->id, (int)pulse, body);
}

/**
* Register code to run when a pin event occurs. 
*/
//% help=pins/on-event weight=20 blockGap=8
//% blockId=pinsonevent block="on|pin %pin|%event"
//% blockNamespace=pins
//% pin.fieldEditor="gridpicker"
//% pin.fieldOptions.width=220
//% pin.fieldOptions.columns=4
void onEvent(DigitalInOutPin pin, PinEvent event, Action body) {
    switch(event) {
        case PinEvent::PulseHigh:
        case PinEvent::PulseLow:
            pin->eventOn(DEVICE_PIN_EVENT_ON_PULSE);
            registerWithDal(pin->id, (int)event, body);
            break;
        case PinEvent::Rise:
        case PinEvent::Fall:
            pin->eventOn(DEVICE_PIN_EVENT_ON_EDGE);
            registerWithDal(pin->id, (int)event, body);
            break;    
    }    
}

/**
 * Measure pin capacitance.
 */
//%
int readCapacitance(DigitalInOutPin pin) {
    // on STM32F4 this requires 2M pulldown
    // other chips to be tested
    pin->setDigitalValue(1);
    pin->getDigitalValue(PullMode::None);
    sleep_us(50);
    return pin->getAnalogValue();
}

/**
* Return the duration of a pulse in microseconds
* @param name the pin which measures the pulse
* @param value the value of the pulse (default high)
* @param maximum duration in micro-seconds
*/
//% blockId="pins_pulse_in" block="pulse in (µs)|pin %name|pulsed %high||timeout %maxDuration (µs)"
//% weight=18 blockGap=8
//% help="pins/pulse-in"
//% blockNamespace=pins
//% pin.fieldEditor="gridpicker"
//% pin.fieldOptions.width=220
//% pin.fieldOptions.columns=4
int pulseIn(DigitalInOutPin pin, PulseValue value, int maxDuration = 2000000) {
    int pulse = PulseValue::High == value ? 1 : 0;
    uint64_t tick = system_timer_current_time_us();
    uint64_t maxd = (uint64_t)maxDuration;
    while (pin->getDigitalValue() != pulse) {
        if (system_timer_current_time_us() - tick > maxd)
            return 0;
    }

    uint64_t start = system_timer_current_time_us();
    while (pin->getDigitalValue() == pulse) {
        if (system_timer_current_time_us() - tick > maxd)
            return 0;
    }
    uint64_t end = system_timer_current_time_us();
    return end - start;
}

/**
* Set the pull direction of this pin.
* @param name pin to set the pull mode on
* @param pull one of the mbed pull configurations: PullUp, PullDown, PullNone
*/
//% help=pins/set-pull weight=17 blockGap=8
//% blockId=device_set_pull block="set pull|pin %pin|to %pull"
//% blockNamespace=pins
//% name.fieldEditor="gridpicker"
//% name.fieldOptions.width=220
//% name.fieldOptions.columns=4
void setPull(DigitalInOutPin name, PinPullMode pull) {
    PullMode m = pull == PinPullMode::PullDown ? PullMode::Down : pull == PinPullMode::PullUp
                                                                        ? PullMode::Up
                                                                        : PullMode::None;
    PINOP(setPull(m));
}

}

namespace pxt {

static void waitABit() {
    // for (int i = 0; i < 10; ++i)
    //    asm volatile("nop");
}

class ButtonMultiplexer : public CodalComponent {
  public:
    Pin &latch;
    Pin &clock;
    Pin &data;
    uint32_t state;
    uint32_t invMask;
    uint16_t buttonIdPerBit[8];
    bool enabled;

    ButtonMultiplexer(uint16_t id)
        : latch(*LOOKUP_PIN(BTNMX_LATCH)), clock(*LOOKUP_PIN(BTNMX_CLOCK)),
          data(*LOOKUP_PIN(BTNMX_DATA)) {
        this->id = id;
        this->status |= DEVICE_COMPONENT_STATUS_SYSTEM_TICK;

        state = 0;
        invMask = 0;
        enabled = true;

        memset(buttonIdPerBit, 0, sizeof(buttonIdPerBit));

        data.getDigitalValue(PullMode::Down);
        latch.setDigitalValue(1);
        clock.setDigitalValue(1);
    }

    void disable() {
        data.getDigitalValue(PullMode::None);
        latch.getDigitalValue(PullMode::None);
        clock.getDigitalValue(PullMode::None);
        enabled = false;
    }

    bool isButtonPressed(int id) {
        for (int i = 0; i < 8; ++i) {
            if (buttonIdPerBit[i] == id)
                return (state & (1 << i)) != 0;
        }
        return false;
    }

    uint32_t readBits(int bits) {
        latch.setDigitalValue(0);
        waitABit();
        latch.setDigitalValue(1);
        waitABit();

        uint32_t state = 0;
        for (int i = 0; i < bits; i++) {
            state <<= 1;
            if (data.getDigitalValue(PullMode::Down))
                state |= 1;

            clock.setDigitalValue(0);
            waitABit();
            clock.setDigitalValue(1);
            waitABit();
        }

        return state;
    }

    virtual void periodicCallback() override {
        if (!enabled)
            return;

        uint32_t newState = readBits(8);
        newState ^= invMask;
        if (newState == state)
            return;

        for (int i = 0; i < 8; ++i) {
            uint32_t mask = 1 << i;
            if (!buttonIdPerBit[i])
                continue;
            int ev = 0;
            if (!(state & mask) && (newState & mask))
                ev = PXT_INTERNAL_KEY_DOWN;
            else if ((state & mask) && !(newState & mask))
                ev = PXT_INTERNAL_KEY_UP;
            if (ev) {
                Event(ev, buttonIdPerBit[i]);
                Event(ev, 0); // any key
            }
        }

        state = newState;
    }
};

static ButtonMultiplexer *btnMultiplexer;
ButtonMultiplexer *getMultiplexer() {
    if (!btnMultiplexer)
        btnMultiplexer = new ButtonMultiplexer(DEVICE_ID_FIRST_BUTTON);
    return btnMultiplexer;
}

int registerMultiplexedButton(int pin, int buttonId) {
    if (1050 <= pin && pin < 1058) {
        pin -= 50;
        getMultiplexer()->invMask |= 1 << (pin - 1000);
    }
    if (1000 <= pin && pin < 1008) {
        getMultiplexer()->buttonIdPerBit[pin - 1000] = buttonId;
        return 1;
    }
    return 0;
}

int multiplexedButtonIsPressed(int btnId) {
    if (btnMultiplexer)
        return btnMultiplexer->isButtonPressed(btnId) ? 512 : 0;
    return 0;
}

//% expose
uint32_t readButtonMultiplexer(int bits) {
    if (!LOOKUP_PIN(BTNMX_CLOCK))
        return 0;
    return getMultiplexer()->readBits(bits);
}

void disableButtonMultiplexer() {
    if (LOOKUP_PIN(BTNMX_CLOCK)) {
        getMultiplexer()->disable();
    }
}

}