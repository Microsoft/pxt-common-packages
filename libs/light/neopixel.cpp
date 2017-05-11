#include "pxt.h"
#include "neopixel.h"

/**
 * Functions to operate colored LEDs.
 */
//% weight=100 color="#0078d7" icon="\uf00a"
namespace light {

    /**
     * Gets the default pin for built in neopixels
     */
    //% parts="neopixel"
    DigitalPin defaultPin() {
        if (PIN_NEOPIXEL == NC)
            return lookupPin(PIN_PA11);
        else
            return lookupPin(PIN_NEOPIXEL);
    }

    /**
     * Sends a neopixel buffer to the specified digital pin
     * @param pin The pin that the neopixels are connected to
     * @param buf The buffer to send to the pin
     */
    //% parts="neopixel"
    void sendBuffer(DigitalPin pin, Buffer buf) {
        neopixel_send_buffer(*pin, buf->payload, buf->length);
    }

}
