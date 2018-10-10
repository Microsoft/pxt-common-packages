#include "pxt.h"
#include "JDProtocol.h"
#include "DMASingleWireSerial.h"

#ifndef CODAL_JACDAC_WIRE_SERIAL
#define CODAL_JACDAC_WIRE_SERIAL DMASingleWireSerial
#endif CODAL_JACDAC_WIRE_SERIAL

namespace jacdac {

// Wrapper classes
class WProtocol {
  public:
    CODAL_JACDAC_WIRE_SERIAL sws;
    codal::JACDAC jd;
    codal::JDProtocol protocol; // note that this is different pins than io->i2c
    WProtocol()
        : sws(PIN(JACDAC))
        , jd(PIN(JACDAC), sws) 
        , protocol(jd)
    {
        jd.init();
    }
};

SINGLETON(WProtocol);

class JDProxyDriver : public JDDriver {
  public:
    RefCollection *methods;

    JDProxyDriver(JDDevice d, RefCollection *m) 
        : JDDriver(d) {
        this->methods = m;
        incrRC(m);
    }

    virtual int handleControlPacket(JDPkt *p) {
        auto buf = pxt::mkBuffer((const uint8_t*)&p->crc, p->size + 4);
        auto r = pxt::runAction1(methods->getAt(0), (TValue)buf);
        auto retVal = numops::toBool(r) ? DEVICE_OK : DEVICE_CANCELLED;
        decr(r);
        decrRC(buf);
        return retVal;
    }

    virtual int handlePacket(JDPkt *p) {
        auto buf = pxt::mkBuffer((const uint8_t*)&p->crc, p->size + 4);
        auto r = pxt::runAction1(methods->getAt(1), (TValue)buf);
        auto retVal = numops::toBool(r) ? DEVICE_OK : DEVICE_CANCELLED;
        decr(r);
        decrRC(buf);
        return retVal;
    }

    virtual int fillControlPacket(JDPkt *p) {
        auto buf = pxt::mkBuffer((const uint8_t*)&p->crc, JD_SERIAL_DATA_SIZE + 4);
        auto r = pxt::runAction1(methods->getAt(2), (TValue)buf);
        memcpy(&p->crc, buf->data, JD_SERIAL_DATA_SIZE + 4);
        (void)r;
        // decr(r); // TODO compiler might return non-null on void functions, so better skip decr
        decrRC(buf);
        return DEVICE_OK;
    }

    virtual int deviceConnected(JDDevice device) {
        auto r = JDDriver::deviceConnected(device);
        pxt::runAction0(methods->getAt(3));
        return r;
    }

    virtual int deviceRemoved() {
        auto r = JDDriver::deviceRemoved();
        pxt::runAction0(methods->getAt(4));
        return r;
    }

    void sendPairing(int address, uint32_t flags, int serialNumber, uint32_t driverClass) {
        sendPairingPacket(JDDevice(address, flags, serialNumber, driverClass));
    }

    JDDevice *getDevice() { return &device; }

    ~JDProxyDriver() { decrRC(methods); }
};

//%
JDProxyDriver *__internalAddDriver(int driverType, int driverClass, RefCollection *methods) {
    getWProtocol();
    return new JDProxyDriver(JDDevice((DriverType)driverType, driverClass), methods);
}

//%
int __internalSendPacket(Buffer buf, int deviceAddress) {
    getWProtocol();
    return JDProtocol::send(buf->data, buf->length, deviceAddress);
}

//%
void start() {
    auto p = getWProtocol();
    if (!p->jd.isRunning())
        p->jd.start();
}

//%
void stop() {
    auto p = getWProtocol();
    if (p->jd.isRunning())
        p->jd.stop();
}

//%
bool isRunning() {
    return getWProtocol()->jd.isRunning();
}

} // namespace jacdac

namespace JacDacDriverStatusMethods {

/**
* Retrieves the serial number in use by this driver.
*
* @return the serial number
**/
//% property
uint32_t serialNumber(JacDacDriverStatus d) {
    return d->getSerialNumber();
}


/** Check if device is paired. */
//% property
bool isPaired(JacDacDriverStatus d) {
    return d->isPaired();
}

/** Check if device is pairable. */
//% property
bool isPairable(JacDacDriverStatus d) {
    return d->isPairable();
}

/** Check if driver is paired. */
//% property
bool isPairedDriver(JacDacDriverStatus d) {
    return d->getDevice()->isPairedDriver();
}

/** Check if driver is connected. */
//% property
bool isConnected(JacDacDriverStatus d) {
    return d->isConnected();
}

/** Get device class. */
//% property
uint32_t driverClass(JacDacDriverStatus d) {
    return d->getClass();
}

/** Get device class. */
//% property
uint8_t driverAddress(JacDacDriverStatus d) {
    return d->getAddress();
}

/** Get device id for events. */
//% property
bool id(JacDacDriverStatus d) {
    return d->id;
}

/** If paired, paired instance address */
//% property
uint32_t isPairedInstanceAddress(JacDacDriverStatus d, uint8_t address) {
    // TODO
    return false;
}

} // namespace JacDacDriverStatusMethods
