#include "pxt.h"

#include "SAMDTCTimer.h"
#include "SAMDTCCTimer.h"
#include "light.h"

namespace pxt {

#ifdef CODAL_JACDAC_WIRE_SERIAL
#ifdef SAMD21
SAMDTCTimer jacdacTimer(TC4, TC4_IRQn);
SAMDTCTimer lowTimer(TC3, TC3_IRQn);

LowLevelTimer* getJACDACTimer()
{
    jacdacTimer.setIRQPriority(1);
    return &jacdacTimer;
}

#endif
#ifdef SAMD51
SAMDTCTimer jacdacTimer(TC0, TC0_IRQn);
SAMDTCTimer lowTimer(TC2, TC2_IRQn);

LowLevelTimer* getJACDACTimer()
{
    jacdacTimer.setIRQPriority(1);
    return &jacdacTimer;
}
#endif
#endif // CODAL_JACDAC_WIRE_SERIAL

__attribute__((used))
CODAL_TIMER devTimer(lowTimer);

static void initRandomSeed() {
    int seed = 0xC0DA1;
    // TODO use TRNG
    seedRandom(seed);
}

void platformSendSerial(const char *data, int len) {}

void platform_init() {
    initRandomSeed();
    setSendToUART(platformSendSerial);
    light::clear();

    /*
        if (*HF2_DBG_MAGIC_PTR == HF2_DBG_MAGIC_START) {
            *HF2_DBG_MAGIC_PTR = 0;
            // this will cause alignment fault at the first breakpoint
            globals[0] = (TValue)1;
        }
    */
}

int *getBootloaderConfigData() {
#ifdef SAMD51
    auto config_data = *(uint32_t *)(BOOTLOADER_END - 4 * 4);
    if (config_data && (config_data & 3) == 0 && config_data < BOOTLOADER_END) {
        auto p = (uint32_t *)config_data;
        if (p[0] == CFG_MAGIC0 && p[1] == CFG_MAGIC1)
            return (int *)p + 4;
    }
#endif
    return NULL;
}

} // namespace pxt

void cpu_clock_init() {}
