#include "pxt.h"
#include "JDProtocol.h"
#include "JackRouter.h"

namespace jacdac {

#ifndef CODAL_JACDAC_WIRE_SERIAL

class DummyDmaSingleWireSerial : public DMASingleWireSerial {
  protected:
    virtual void configureRxInterrupt(int enable) {}
    virtual int configureTx(int) { return DEVICE_OK; }
    virtual int configureRx(int) { return DEVICE_OK; }

  public:
    DummyDmaSingleWireSerial(Pin &p) : DMASingleWireSerial(p) {}

    virtual int sendDMA(uint8_t *data, int len) { return DEVICE_OK; }
    virtual int receiveDMA(uint8_t *data, int len) { return DEVICE_OK; }
    virtual int abortDMA() { return DEVICE_OK; }

    virtual int putc(char c) { return DEVICE_OK; }
    virtual int getc() { return DEVICE_OK; }

    virtual int send(uint8_t *buf, int len) { return DEVICE_OK; }
    virtual int receive(uint8_t *buf, int len) { return DEVICE_OK; }

    virtual int setBaud(uint32_t baud) { return DEVICE_OK; }
    virtual uint32_t getBaud() { return 0; }
    virtual int sendBreak() { return DEVICE_OK; }
};

#define CODAL_JACDAC_WIRE_SERIAL DummyDmaSingleWireSerial
#endif

// Wrapper classes
class WProtocol {
  public:
    CODAL_JACDAC_WIRE_SERIAL sws;
    codal::JACDAC jd;
    codal::JDProtocol protocol; // note that this is different pins than io->i2c
    codal::JackRouter *jr;
    WProtocol()
        : sws(*LOOKUP_PIN(JACK_TX))
        , jd(*LOOKUP_PIN(JACK_TX), sws)
        , protocol(jd) {
        if (LOOKUP_PIN(JACK_HPEN)) {
            jr = new codal::JackRouter(*LOOKUP_PIN(JACK_TX), *LOOKUP_PIN(JACK_SENSE),
                                       *LOOKUP_PIN(JACK_HPEN), *LOOKUP_PIN(JACK_BZEN),
                                       *LOOKUP_PIN(JACK_PWREN), jd);
        } else {
            jr = NULL;
        }
        jd.start();
    }
};
SINGLETON(WProtocol);

void setJackRouterOutput(int output) {
    auto jr = getWProtocol()->jr;
    if (!jr)
        return;
    if (output < 0)
        return;
    switch (output) {
    case 0:
        jr->forceState(JackState::None);
        break;
    case 1:
        jr->forceState(JackState::BuzzerAndSerial);
        break;
    case 2:
        jr->forceState(JackState::HeadPhones);
        break;
    }
}

/**
 * Starts the JacDac protocol
 */
//%
void start() {
    auto p = getWProtocol();
    if (!p->jd.isRunning())
        p->jd.start();
}

/**
 * Starts the JacDac protocol
 */
//%
void stop() {
    auto p = getWProtocol();
    if (p->jd.isRunning())
        p->jd.stop();
}

class JDProxyDriver : public JDDriver {
  public:
    RefCollection *methods;

    JDProxyDriver(JDDevice d, RefCollection *m) : JDDriver(d) {
        this->methods = m;
        incrRC(m);
        registerGCPtr((TValue)m);
    }

    virtual int handleControlPacket(JDPkt *p) {
        ControlPacket* cp = (ControlPacket*)p->data;
        if (this->device.isPairedDriver() && !this->device.isPaired())
        {
            DMESG("NEED TO PAIR!");
            if (cp->flags & CONTROL_JD_FLAGS_PAIRABLE)
            {
                DMESG("PAIR!");
                sendPairingPacket(JDDevice(cp->address, JD_DEVICE_FLAGS_REMOTE | JD_DEVICE_FLAGS_INITIALISED | JD_DEVICE_FLAGS_CP_SEEN, cp->serial_number, cp->driver_class));
            }
        }
        return DEVICE_OK;
    }

    virtual int handlePacket(JDPkt *p) {
        auto buf = pxt::mkBuffer((const uint8_t *)&p->crc, p->size + 4);
        auto r = pxt::runAction1(methods->getAt(0), (TValue)buf);
        auto retVal = numops::toBool(r) ? DEVICE_OK : DEVICE_CANCELLED;
        decr(r);
        decrRC(buf);
        return retVal;
    }

    bool isPairedInstanceAddress(uint8_t address) {
        return NULL != this->pairedInstance && this->pairedInstance->getAddress() == address;
    }

    JDDevice *getDevice() { return &device; }

    ~JDProxyDriver() {
        decrRC(methods);
        unregisterGCPtr((TValue)methods);
    }
};

//%
JDProxyDriver *__internalAddDriver(int driverType, int driverClass, RefCollection *methods) {
    getWProtocol();
    return new JDProxyDriver(JDDevice((DriverType)driverType, driverClass), methods);
}

/**
 * Internal
 */
//%
int __internalSendPacket(Buffer buf, int deviceAddress) {
    getWProtocol();
    return JDProtocol::send(buf->data, buf->length, deviceAddress);
}

} // namespace jacdac

namespace JacDacDriverStatusMethods {

/**
* Returns the JDDevice instnace
*/
//% property
Buffer device(JacDacDriverStatus d) {
    return pxt::mkBuffer((const uint8_t *)d->getDevice(), sizeof(JDDevice));
}

/** Check if driver is connected. */
//% property
bool isConnected(JacDacDriverStatus d) {
    return d->isConnected();
}

/** Get device id for events. */
//% property
uint32_t id(JacDacDriverStatus d) {
    return d->id;
}

/** If paired, paired instance address */
//% property
bool isPairedInstanceAddress(JacDacDriverStatus d, uint8_t address) {
    return d->isPairedInstanceAddress(address);
}

} // namespace JacDacDriverStatusMethods
