#include "pxt.h"
#include "ST7735.h"

namespace pxt {
class WDisplay {
  public:
    CODAL_SPI spi;
    ST7735 lcd;

    uint32_t currPalette[16];
    bool newPalette;

    uint8_t *screenBuf;
    Image_ lastImg;

    int width, height;
    uint32_t palXOR;

    WDisplay()
        : spi(*LOOKUP_PIN(DISPLAY_MOSI), *LOOKUP_PIN(DISPLAY_MISO), *LOOKUP_PIN(DISPLAY_SCK)),
          lcd(spi, *LOOKUP_PIN(DISPLAY_CS), *LOOKUP_PIN(DISPLAY_DC)) {

        auto rst = LOOKUP_PIN(DISPLAY_RST);
        if (rst) {
            rst->setDigitalValue(0);
            fiber_sleep(20);
            rst->setDigitalValue(1);
            fiber_sleep(20);
        }

        auto bl = LOOKUP_PIN(DISPLAY_BL);
        if (bl) {
            bl->setDigitalValue(1);
        }

        uint32_t cfg0 = getConfig(CFG_DISPLAY_CFG0, 0x40);
        uint32_t cfg2 = getConfig(CFG_DISPLAY_CFG2, 0x0);
        uint32_t frmctr1 = getConfig(CFG_DISPLAY_CFG1, 0x000603);
        palXOR = (cfg0 & 0x1000000) ? 0xffffff : 0x000000;
        auto madctl = cfg0 & 0xff;
        auto offX = (cfg0 >> 8) & 0xff;
        auto offY = (cfg0 >> 16) & 0xff;
        auto freq = (cfg2 & 0xff);
        if (!freq)
            freq = 15;

        DMESG("configure screen: FRMCTR1=%p MADCTL=%p SPI at %dMHz", frmctr1, madctl, freq);

        spi.setFrequency(freq * 1000000);
        spi.setMode(0);
        lcd.init();
        lcd.configure(madctl, frmctr1);
        width = getConfig(CFG_DISPLAY_WIDTH, 160);
        height = getConfig(CFG_DISPLAY_HEIGHT, 128);
        lcd.setAddrWindow(offX, offY, width, height);
        DMESG("screen: %d x %d, off=%d,%d", width, height, offX, offY);
        screenBuf = new uint8_t[width * height / 2 + 20];
        lastImg = NULL;
    }
};

SINGLETON(WDisplay);

//%
void setPalette(Buffer buf) {
    auto display = getWDisplay();
    if (48 != buf->length)
        target_panic(PANIC_SCREEN_ERROR);
    for (int i = 0; i < 16; ++i) {
        display->currPalette[i] =
            (buf->data[i * 3] << 16) | (buf->data[i * 3 + 1] << 8) | (buf->data[i * 3 + 2] << 0);
        display->currPalette[i] ^= display->palXOR;
    }
    display->newPalette = true;
}

static const uint8_t numbers[] = {
    0x06, 0x09, 0x09, 0x09, 0x06, // 0
    0x04, 0x06, 0x04, 0x04, 0x0e, // 1
    0x07, 0x08, 0x06, 0x01, 0x0f, // 2
    0x0f, 0x08, 0x04, 0x09, 0x06, // 3
    0x0c, 0x0a, 0x09, 0x1f, 0x08, // 4
    0x1f, 0x01, 0x0f, 0x10, 0x0f, // 5
    0x08, 0x04, 0x0e, 0x11, 0x0e, // 6
    0x1f, 0x08, 0x04, 0x02, 0x01, // 7
    0x0e, 0x11, 0x0e, 0x11, 0x0e, // 8
    0x0e, 0x11, 0x0e, 0x04, 0x02, // 9
    0x11, 0x00, 0x0e, 0x1b, 0x11, // :(
    // 0x11, 0x04, 0x04, 0x0a, 0x11, // :(
};

static void drawNumber(int idx, uint8_t *bmp, int x, int y, int hb) {
    const uint8_t *src = &numbers[idx * 5];
    y >>= 1;
    auto mask = idx == 10 ? 0x22 : 0x11;
    for (int i = 0; i < 5; i++) {
        uint8_t ch = *src++;
        for (int j = 0; j < 5; j++) {
            if (ch & (1 << j)) {
                for (int k = 0; k < 4; ++k) {
                    auto p = bmp + (x + (j * 4) + k) * hb + (y + i * 2);
                    *p++ = mask;
                    *p++ = mask;
                }
            }
        }
    }
}

static void drawPanic(int code) {
    auto display = getWDisplay();
    auto hb = display->width >> 1;
    auto ptr = display->screenBuf;
    auto dw = display->height;

    memset(ptr, 0, hb * dw);

    drawNumber(10, ptr, 70, 20, hb);
    int x = 50;
    int y = 60;
    drawNumber((code / 100) % 10, ptr, x, y, hb);
    x += 24;
    drawNumber((code / 10) % 10, ptr, x, y, hb);
    x += 24;
    drawNumber((code / 1) % 10, ptr, x, y, hb);
    x += 24;

    display->lcd.waitForSendDone();
    display->lcd.sendIndexedImage(display->screenBuf, display->width, display->height, NULL);
    display->lcd.waitForSendDone();
}

extern "C" void target_panic(int statusCode) {
    DMESG("*** CODAL PANIC : [%d]", statusCode);

    drawPanic(statusCode);

    while (1) {
    }
}

//%
void updateScreen(Image_ img) {
    auto display = getWDisplay();

    if (img && img != display->lastImg) {
        decrRC(display->lastImg);
        incrRC(img);
        display->lastImg = img;
    }
    img = display->lastImg;

    if (img && img->isDirty()) {
        if (img->bpp() != 4 || img->width() != display->width || img->height() != display->height)
            target_panic(PANIC_SCREEN_ERROR);

        img->clearDirty();
        // DMESG("wait for done");
        display->lcd.waitForSendDone();

        auto palette = display->currPalette;

        if (display->newPalette) {
            display->newPalette = false;
        } else {
            palette = NULL;
        }

        memcpy(display->screenBuf, img->pix(), img->pixLength());

        // DMESG("send");
        display->lcd.sendIndexedImage(display->screenBuf, display->width, display->height, palette);
    }
}

//%
void updateStats(String msg) {
    // ignore...
}
} // namespace pxt