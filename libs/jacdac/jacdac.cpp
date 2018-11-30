#include "pxt.h"
#include "JDProtocol.h"
#include "JackRouter.h"

#define JD_DRIVER_EVT_FILL_CONTROL_PACKET 50

namespace jacdac {

// Wrapper classes
class WProtocol {
#ifdef CODAL_JACDAC_WIRE_SERIAL
    CODAL_JACDAC_WIRE_SERIAL sws;
    codal::JACDAC jd;
    codal::JDProtocol protocol; // note that this is different pins than io->i2c
    codal::JackRouter *jr;
#endif    
  public:
    WProtocol()
#ifdef CODAL_JACDAC_WIRE_SERIAL
        : sws(*LOOKUP_PIN(JACK_TX))
        , jd(sws)
        , protocol(jd) 
#endif
        {
#ifdef CODAL_JACDAC_WIRE_SERIAL
        if (LOOKUP_PIN(JACK_HPEN)) {
            jr = new codal::JackRouter(*LOOKUP_PIN(JACK_TX), *LOOKUP_PIN(JACK_SENSE),
                                       *LOOKUP_PIN(JACK_HPEN), *LOOKUP_PIN(JACK_BZEN),
                                       *LOOKUP_PIN(JACK_PWREN), jd);
        } else {
            jr = NULL;
        }
        jd.start();
#endif       
    }

    void start() {
#ifdef CODAL_JACDAC_WIRE_SERIAL
    if (!jd.isRunning())
        jd.start();
#endif
    }

    void stop() {
#ifdef CODAL_JACDAC_WIRE_SERIAL
    if (jd.isRunning())
        jd.stop();
#endif
    }

    void setBridge(JDDriver* driver) {
#ifdef CODAL_JACDAC_WIRE_SERIAL
        protocol.setBridge(*driver);
#endif
    }

    void setJackRouterOutput(int output) {
    #ifdef CODAL_JACDAC_WIRE_SERIAL
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
    #endif
    }

};
SINGLETON(WProtocol);

void setJackRouterOutput(int output) {
    getWProtocol()->setJackRouterOutput(output);
}

/**
 * Starts the JacDac protocol
 */
//% parts=jacdac
void start() {
    getWProtocol()->start();
}

/**
 * Starts the JacDac protocol
 */
//% parts=jacdac
void stop() {
    getWProtocol()->stop();
}

/**
* Clears any existing bridge
*/
//% parts=jacdac
void clearBridge() {
    // TODO
  //  auto p = getWProtocol();
//    p->protocol.setBridge(NULL);
}

#ifdef CODAL_JACDAC_WIRE_SERIAL
class JDProxyDriver : public JDDriver 
{
  public:
    RefCollection *methods;
    Buffer _controlData; // may be NULL

    JDProxyDriver(JDDevice d, RefCollection *m, Buffer controlData) 
        : JDDriver(d)
        , methods(m)
        , _controlData(controlData) {
        incrRC(this->methods);
        registerGCPtr((TValue)this->methods);
        if (this->_controlData) {
            incrRC(this->_controlData);
            registerGCPtr((TValue)this->_controlData);
        }
    }

    virtual int fillControlPacket(JDPkt* p) {
        if (NULL != _controlData && _controlData->length) {
            ControlPacket* cp = (ControlPacket*)p->data;
            auto n = min(CONTROL_PACKET_PAYLOAD_SIZE, this->_controlData->length);
            memcpy(cp->data, this->_controlData->data, n);
            Event(this->id, JD_DRIVER_EVT_FILL_CONTROL_PACKET);
        }
        return DEVICE_OK;
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

        auto buf = pxt::mkBuffer((const uint8_t *)cp, sizeof(ControlPacket));
        auto r = pxt::runAction1(methods->getAt(1), (TValue)buf);
        auto retVal = numops::toBool(r) ? DEVICE_OK : DEVICE_CANCELLED;
        decr(r);
        decrRC(buf);        
        return retVal;
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
        if (_controlData) {
            decrRC(_controlData);
            unregisterGCPtr((TValue)_controlData);
        }
    }
};

#else
class JDProxyDriver {};
#endif

typedef JDProxyDriver* JacDacDriverStatus;
typedef RefCollection* MethodCollection;
/**
Internal
*/
//% parts=jacdac
JacDacDriverStatus __internalAddDriver(int driverType, int driverClass, MethodCollection methods, Buffer controlData) {
    getWProtocol();
#ifdef CODAL_JACDAC_WIRE_SERIAL
    return new JDProxyDriver(JDDevice((DriverType)driverType, driverClass), methods, controlData);
#else
    return new JDProxyDriver();
#endif
}

/**
 * Internal
 */
//% parts=jacdac
int __internalSendPacket(Buffer buf, int deviceAddress) {
    getWProtocol();
    return JDProtocol::send(buf->data, buf->length, deviceAddress);
}

} // namespace jacdac

namespace JacDacDriverStatusMethods {

/**
* Returns the JDDevice instance
*/
//% property
Buffer device(JacDacDriverStatus d) {
#ifdef CODAL_JACDAC_WIRE_SERIAL
    return pxt::mkBuffer((const uint8_t *)d->getDevice(), sizeof(JDDevice));
#else
    return NULL;
#endif
}

/** Check if driver is connected. */
//% property
bool isConnected(JacDacDriverStatus d) {
#ifdef CODAL_JACDAC_WIRE_SERIAL
    return d->isConnected();
#else
    return false;
#endif   
}

/** Get device id for events. */
//% property
uint32_t id(JacDacDriverStatus d) {
#ifdef CODAL_JACDAC_WIRE_SERIAL
    return d->id;
#else
    return 999;
#endif
}

/** If paired, paired instance address */
//% property
bool isPairedInstanceAddress(JacDacDriverStatus d, uint8_t address) {
#ifdef CODAL_JACDAC_WIRE_SERIAL
    return d->isPairedInstanceAddress(address);
#else
    return false;
#endif
}

/**
* Set driver as bridge
*/
//%
void setBridge(JacDacDriverStatus d) {
#ifdef CODAL_JACDAC_WIRE_SERIAL
    jacdac::getWProtocol()->setBridge(d);
#endif    
}

} // namespace JacDacDriverStatusMethods
