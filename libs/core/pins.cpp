#include "pxt.h"

enum class PulseValue {
    //% block=high
    High = DEVICE_PIN_EVT_PULSE_HI,
    //% block=low
    Low = DEVICE_PIN_EVT_PULSE_LO
};

enum class PinPullMode {
    //% block="down"
    PullDown = 0,
    //% block="up"
    PullUp = 1,
    //% block="none"
    PullNone = 2
};

namespace pxt {
//%
DevicePin *getPin(int id) {
    if (!(0 <= id && id <= LastPinID))
        target_panic(42);
    DevicePin *p = &io->pins[id];
    // if (p->name == NC)
    //    return NULL;
    return p;
}

#pragma GCC diagnostic ignored "-Warray-bounds"

//%
DevicePin *lookupPin(int pinName) {
    for (int i = 0; i <= LastPinID; ++i) {
        if (io->pins[i].name == pinName)
            return &io->pins[i];
    }
    return NULL;
}
}


namespace DigitalPinMethods {
/**
 * Read a pin or connector as either 0 or 1
 * @param name pin to read from
 */
//% help=pins/digital-read weight=61
//% blockId=device_get_digital_pin block="digital read|pin %name" blockGap=8
//% parts="slideswitch" trackArgs=0
//% blockNamespace=pins
//% name.fieldEditor="gridpicker"
//% name.fieldOptions.width=220
//% name.fieldOptions.columns=4
bool digitalRead(DigitalPin name) {
    return PINOP(getDigitalValue()) != 0;
}

/**
  * Set a pin or connector value to either 0 or 1.
  * @param name pin to write to
  * @param value value to set on the pin
  */
//% help=pins/digital-write weight=60
//% blockId=device_set_digital_pin block="digital write|pin %name|to %value"
//% parts="led" trackArgs=0
//% blockNamespace=pins
//% name.fieldEditor="gridpicker"
//% name.fieldOptions.width=220 
//% name.fieldOptions.columns=4
void digitalWrite(DigitalPin name, bool value) {
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
void onPulsed(DigitalPin pin, PulseValue pulse, Action body) {
    pin->eventOn(DEVICE_PIN_EVENT_ON_PULSE);
    registerWithDal(pin->id, (int)pulse, body);
}

/**
* Return the duration of a pulse in microseconds
* @param name the pin which measures the pulse
* @param value the value of the pulse (default high)
* @param maximum duration in micro-seconds
*/
//% blockId="pins_pulse_in" block="pulse in (µs)|pin %name|pulsed %value"
//% weight=18 blockGap=8
//% help="pins/pulse-in"
//% blockNamespace=pins
//% pin.fieldEditor="gridpicker"
//% pin.fieldOptions.width=220
//% pin.fieldOptions.columns=4
int pulseIn(DigitalPin pin, PulseValue value, int maxDuration = 2000000) {
    int pulse = value == PulseValue::High ? 1 : 0;
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
void setPull(DigitalPin name, PinPullMode pull) {
    PullMode m = pull == PinPullMode::PullDown ? PullMode::Down : pull == PinPullMode::PullUp
                                                                        ? PullMode::Up
                                                                        : PullMode::None;
    PINOP(setPull(m));
}

}

namespace AnalogPinMethods {

/**
 * Read the connector value as analog, that is, as a value comprised between 0 and 1023.
 * @param name pin to write to
 */
//% help=pins/analog-read weight=53
//% blockId=device_get_analog_pin block="analog read|pin %name" blockGap="8"
//% blockNamespace=pins
//% name.fieldEditor="gridpicker"
//% name.fieldOptions.width=220
//% name.fieldOptions.columns=4
int analogRead(AnalogPin name) {
    return PINOP(getAnalogValue());
}

/**
 * Set the connector value as analog. Value must be comprised between 0 and 1023.
 * @param name pin name to write to
 * @param value value to write to the pin between ``0`` and ``1023``. eg:1023,0
 */
//% help=pins/analog-write weight=52
//% blockId=device_set_analog_pin block="analog write|pin %name|to %value" blockGap=8
//% blockNamespace=pins
//% name.fieldEditor="gridpicker"
//% name.fieldOptions.width=220
//% name.fieldOptions.columns=4
void analogWrite(AnalogPin name, int value) {
    PINOP(setAnalogValue(value));
}
}

namespace PwmPinMethods {

/**
 * Set the Pulse-width modulation (PWM) period of the analog output. The period is in
 * **microseconds** or `1/1000` milliseconds.
 * If this pin is not configured as an analog output (using `analog write pin`), the operation has
 * no effect.
 * @param name analog pin to set period to
 * @param micros period in micro seconds. eg:20000
 */
//% help=pins/analog-set-period weight=51
//% blockId=device_set_analog_period block="analog set period|pin %pin|to (µs)%period"
//% blockNamespace=pins
//% name.fieldEditor="gridpicker"
//% name.fieldOptions.width=220
//% name.fieldOptions.columns=4
void analogSetPeriod(PwmPin name, int period) {
    PINOP(setAnalogPeriodUs(period));
}

/**
 * Write a value to the servo to control the rotation of the shaft. On a standard servo, this will
 * set the angle of the shaft (in degrees), moving the shaft to that orientation. On a continuous
 * rotation servo, this will set the speed of the servo (with ``0`` being full-speed in one
 * direction, ``180`` being full speed in the other, and a value near ``90`` being no movement).
 * @param name pin to write to
 * @param value angle or rotation speed, eg:180,90,0
 */
//% help=pins/servo-write weight=41 group="Servo"
//% blockId=device_set_servo_pin block="servo write|pin %name|to %value" blockGap=8
//% parts=microservo trackArgs=0
//% blockNamespace=pins
//% name.fieldEditor="gridpicker"
//% name.fieldOptions.width=220
//% name.fieldOptions.columns=4
void servoWrite(PwmPin name, int value) {
    PINOP(setServoValue(value));
}

/**
 * Set the pin for PWM analog output, make the period be 20 ms, and set the pulse width.
 * The pulse width is based on the value it is given **microseconds** or `1/1000` milliseconds.
 * @param name pin name
 * @param duration pulse duration in micro seconds, eg:1500
 */
//% help=pins/servo-set-pulse weight=40 group="Servo" blockGap=8
//% blockId=device_set_servo_pulse block="servo set pulse|pin %value|to (µs) %duration"
//% blockNamespace=pins
//% name.fieldEditor="gridpicker"
//% name.fieldOptions.width=220
//% name.fieldOptions.columns=4
void servoSetPulse(PwmPin name, int duration) {
    PINOP(setServoPulseUs(duration));
}

}

namespace pins {
/**
 * Create a new zero-initialized buffer.
 * @param size number of bytes in the buffer
 */
//%
Buffer createBuffer(int size) {
    return mkBuffer(NULL, size);
}

/**
* Get the duration of the last pulse in microseconds. This function should be called from a
* ``onPulsed`` handler.
*/
//% help=pins/pulse-duration blockGap=8
//% blockId=pins_pulse_duration block="pulse duration (µs)"
//% weight=19
int pulseDuration() {
    return pxt::lastEvent.timestamp;
}
}
