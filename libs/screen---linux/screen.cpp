#include "pxt.h"
#include "pins.h"
namespace pxt {
class WDisplay {
  public:
    uint32_t currPalette[16];
    bool newPalette;

    uint8_t *screenBuf;
    Image_ lastImg;

    int width, height;

    WDisplay() {
        width = getConfig(CFG_DISPLAY_WIDTH, 160);
        height = getConfig(CFG_DISPLAY_HEIGHT, 128);
        screenBuf = new uint8_t[width * height / 2 + 20];
        lastImg = NULL;
        newPalette = false;
    }
};

SINGLETON(WDisplay);

//%
void setPalette(Buffer buf) {
    auto display = getWDisplay();
    if (48 != buf->length)
        target_panic(907);
    for (int i = 0; i < 16; ++i) {
        display->currPalette[i] =
            (buf->data[i * 3] << 16) | (buf->data[i * 3 + 1] << 8) | (buf->data[i * 3 + 2] << 0);
    }
    display->newPalette = true;
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
            target_panic(906);

        img->clearDirty();

        #if 0
        //DMESG("wait for done");
        display->lcd.waitForSendDone();
        
        auto palette = display->currPalette;

        if (display->newPalette) {
            display->newPalette = false;
        } else {
            palette = NULL;
        }

        memcpy(display->screenBuf, img->pix(), img->pixLength());

        //DMESG("send");
        display->lcd.sendIndexedImage(display->screenBuf, display->width, display->height, palette);
        #endif
    }
}

//%
void updateStats(String msg) {
    // ignore...
}
} // namespace pxt