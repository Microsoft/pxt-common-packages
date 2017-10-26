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
        int pinName = PIN(NEOPIXEL);
        if (pinName < 0) {
            pinName = PA11;
        }
        
        return lookupPin(pinName);
    }

    /**
     * Sends a neopixel buffer to the specified digital pin
     * @param pin The pin that the neopixels are connected to
     * @param mode the color encoding mode
     * @param buf The buffer to send to the pin
     */
    //% parts="neopixel"
    void sendBuffer(DigitalPin pin, int mode, Buffer buf) {
        neopixel_send_buffer(*pin, buf->data, buf->length);
    }

}
