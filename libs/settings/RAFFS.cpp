#include "RAFFS.h"
#include "CodalDmesg.h"
#include "NotifyEvents.h"
#include "MessageBus.h"
#include <stddef.h>

#define RAFFS_MAGIC 0x6776e0da
#define M1 0xffffffffU

using namespace codal;

#define oops() target_panic(DEVICE_FLASH_ERROR)
#define ASSERT(cond)                                                                               \
    do {                                                                                           \
        if (!(cond))                                                                               \
            oops();                                                                                \
    } while (0)

#define OFF2(v, basePtr) (uint32_t)((uint32_t *)v - (uint32_t *)basePtr)
#define OFF(v) OFF2(v, basePtr)

#undef NOLOG
#define NOLOG(...) ((void)0)
#define LOG DMESG
#define LOGV NOLOG
#define LOGVV NOLOG

#if 0
#undef LOGV
#define LOGV DMESG
#endif

#if 0
#undef LOGVV
#define LOGVV DMESG
#endif

using namespace pxt::raffs;

static uint16_t raffs_unlocked_event;

struct FSHeader {
    uint32_t magic;
    uint32_t bytes;
};

// https://tools.ietf.org/html/draft-eastlake-fnv-14#section-3
static uint16_t fnhash(const char *fn) {
    uint32_t h = 0x811c9dc5;
    while (*fn)
        h = (h * 0x1000193) ^ (uint8_t)*fn++;
    return h ^ (h >> 16);
}

FS::FS(Flash &flash, uintptr_t baseAddr, uint32_t bytes)
    : flash(flash), baseAddr(baseAddr), bytes(bytes) {
    files = NULL;
    locked = false;

    basePtr = NULL;
    endPtr = NULL;
    freeDataPtr = NULL;
    metaPtr = NULL;
    readDirPtr = NULL;
    cachedMeta = NULL;
    flashBufAddr = 0;

    if (bytes > 0x20000)
        oops();

    auto page = flash.pageSize(baseAddr);
    // baseAddr and bytes needs to page-aligned, and we need even number of pages
    auto numPages = bytes / page;
    if ((baseAddr & (page - 1)) || bytes % page || numPages < 2 || (numPages & 1))
        oops();

    if (!raffs_unlocked_event)
        raffs_unlocked_event = codal::allocateNotifyEvent();
}

void FS::erasePages(uintptr_t addr, uint32_t len) {
    auto end = addr + len;
    auto page = flash.pageSize(addr);
    if (addr & (page - 1))
        oops();
    while (addr < end) {
        if (flash.pageSize(addr) != page)
            oops();
        flash.erasePage(addr);
        addr += page;
    }
}

void FS::flushFlash() {
    if (flashBufAddr) {
        flash.writeBytes(flashBufAddr, flashBuf, sizeof(flashBuf));
        for (unsigned i = 0; i < sizeof(flashBuf); ++i)
            if (flashBuf[i] != 0xff && flashBuf[i] != ((uint8_t *)flashBufAddr)[i])
                target_panic(999);
        flashBufAddr = 0;
    }
}

void FS::writeBytes(void *dst, const void *src, uint32_t size) {
    LOGV("write %x%s %d %x:%x:%x:%x",
         OFF(dst) <= OFF2(dst, altBasePtr()) ? OFF(dst) : OFF2(dst, altBasePtr()),
         OFF(dst) <= OFF2(dst, altBasePtr()) ? "" : "*", size, ((const uint8_t *)src)[0],
         ((const uint8_t *)src)[1], ((const uint8_t *)src)[2], ((const uint8_t *)src)[3]);

    while (size > 0) {
        uint32_t off = (uintptr_t)dst & (sizeof(flashBuf) - 1);
        uintptr_t newaddr = (uintptr_t)dst - off;
        if (newaddr != flashBufAddr) {
            flushFlash();
            memset(flashBuf, 0xff, sizeof(flashBuf));
            flashBufAddr = newaddr;
        }

        unsigned n = sizeof(flashBuf) - off;
        if (n > size)
            n = size;
        memcpy(flashBuf + off, src, n);
        size -= n;
        src = (const uint8_t *)src + n;
        dst = (uint8_t *)dst + n;
    }
}

void FS::format() {
    if (files)
        oops();

    LOG("formatting...");

    FSHeader hd;

    // in case the secondary header is valid, clear it
    auto hd2 = (FSHeader *)(baseAddr + bytes / 2);
    if (hd2->magic == RAFFS_MAGIC) {
        hd.magic = 0;
        hd.bytes = 0;
        writeBytes(hd2, &hd, sizeof(hd));
    }

    // write the primary header
    erasePages(baseAddr, bytes / 2);
    hd.magic = RAFFS_MAGIC;
    hd.bytes = bytes;
    writeBytes((void *)baseAddr, &hd, sizeof(hd));

    flushFlash();
}

#define NUMBLOCKED (sizeof(blocked->fnptr) / sizeof(uint16_t))

bool FS::checkBlocked(MetaEntry *m) {
    auto fnptr = m->fnptr;
    for (auto p = blocked; p; p = p->next) {
        for (int i = 0; i < NUMBLOCKED; ++i)
            if (p->fnptrs[i] == fnptr) {
                if (m.isFirst())
                    p->fnptrs[i] = 0;
                return true;
            }
    }
    if (!m.isFirst()) {
        for (auto p = blocked; p; p = p->next) {
            for (int i = 0; i < NUMBLOCKED; ++i)
                if (p->fnptrs[i] == 0) {
                    p->fnptr[i] = fnptr;
                    return;
                }
        }
        auto p = new BlockedEntries;
        memset(p, 0, sizeof(*p));
        p->next = blocked;
        blocked = p;
        p->fnptrs[0] = fnptr;
    }
    return false;
}

void FS::clearBlocked() {
    while (blocked) {
        auto p = blocked;
        blocked = p->next;
        delete p;
    }
}

bool FS::tryMount() {
    if (basePtr)
        return true;

    auto addr = baseAddr + bytes / 2;

    auto hd = (FSHeader *)addr;
    if (hd->magic == RAFFS_MAGIC && hd->bytes == bytes) {
        // OK
    } else {
        addr = baseAddr;
        hd = (FSHeader *)addr;
        if (hd->magic == RAFFS_MAGIC && hd->bytes == bytes) {
            // OK
        } else {
            return false;
        }
    }

    basePtr = (uint8_t *)addr;
    endPtr = (MetaEntry *)(addr + bytes / 2);

    auto p = (uint32_t *)endPtr - 2;
    while (*p != M1)
        p -= 2;
    metaPtr = (MetaEntry *)(p + 2);

    p = (uint32_t *)metaPtr - 1;
    while (*p == M1)
        p--;
    freeDataPtr = RAFFS_ROUND(p + 1);

    auto fp = (uint32_t *)freeDataPtr;
    if (fp[0] != M1 || fp[1] != M1)
        oops();

    LOG("mounted, end=%x meta=%x free=%x", OFF(endPtr), OFF(metaPtr), OFF(freeDataPtr));

    return true;
}

void FS::mount() {
    // if (basePtr) return;
    if (tryMount())
        return;
    format();
    if (!tryMount())
        oops();
}

FS::~FS() {}

int FS::write(const char *keyName, const uint8_t *data, uint32_t bytes) {
    auto isDel = data == NULL && bytes == M1;
    if (!isDel && !data && bytes)
        oops();

    lock();
    uint32_t szneeded = bytes;
    auto existing = findMetaEntry(keyName);
    auto prevBase = basePtr;

    cachedMeta = NULL;

    if (!existing) {
        if (isDel) {
            unlock();
            return -1;
        }
        szneeded += strlen(keyName) + 1;
    }

    if (!tryGC(sizeof(MetaEntry) + RAFFS_ROUND(szneeded))) {
        unlock();
        return -1;
    }

    // if the GC happened, find the relocated meta entry
    if (prevBase != basePtr)
        existing = findMetaEntry(keyName);

    MetaEntry newMeta;
    if (existing) {
        newMeta.fnhash = existing->fnhash;
        newMeta.fnptr = existing->fnptr;
    } else {
        newMeta.fnhash = fnhash(keyName);
        newMeta.fnptr = writeData(keyName, strlen(keyName) + 1);
    }
    newMeta.dataptr = isDel ? 0 : writeData(data, bytes);
    newMeta._datasize = bytes;
    if (existing)
        newMeta._datasize |= RAFFS_FOLLOWING_MASK;
    finishWrite();

    writeBytes(--metaPtr, &newMeta, sizeof(newMeta));
    flushFlash();

    unlock();
    return 0;
}

int FS::read(const char *keyName, uint8_t *data, uint32_t bytes) {
    lock();
    int r = -1;
    auto meta = keyName ? findMetaEntry(keyName) : cachedMeta;
    if (meta != NULL && meta->dataptr) {
        r = meta->datasize();
        if (data) {
            if (bytes > r)
                bytes = r;
            memcpy(data, basePtr + meta->dataptr, bytes);
        }
    }
    unlock();
    return r;
}

int FS::remove(const char *keyName) {
    return write(keyName, NULL, M1);
}

void FS::lock() {
    while (locked)
        fiber_wait_for_event(DEVICE_ID_NOTIFY, raffs_unlocked_event);
    locked = true;
    mount();
}

void FS::unlock() {
    if (!locked)
        oops();
    flushFlash();
    locked = false;
#ifndef RAFFS_TEST
    Event(DEVICE_ID_NOTIFY, raffs_unlocked_event);
#endif
}

MetaEntry *FS::findMetaEntry(const char *filename) {
    uint16_t h = fnhash(filename);
    uint16_t buflen = strlen(filename) + 1;

    for (auto p = metaPtr; p < endPtr; p++) {
        // LOGV("check at %x %x %x", OFF(p),p->fnhash,h);
        if (p->fnhash == h && memcmp(basePtr + p->fnptr, filename, buflen) == 0)
            return p;
    }

    // LOGV("fail");

    return NULL;
}

void FS::forceGC(filename_filter filter) {
    lock();
    tryGC(0x7fff0000, filter);
    unlock();
}

bool FS::tryGC(int spaceNeeded, filename_filter filter) {
    int spaceLeft = (intptr_t)metaPtr - (intptr_t)freeDataPtr;

#ifdef RAFFS_TEST
    for (auto p = (uint32_t *)freeDataPtr; p < (uint32_t *)metaPtr; p++) {
        if (*p != M1) {
            LOG("value at %x = %x", OFF(p), *p);
            oops();
        }
    }
#endif

    if (spaceLeft > spaceNeeded + 32)
        return true;

    LOG("running flash FS GC; needed %d, left %d", spaceNeeded, spaceLeft);

    readDirPtr = NULL;
    cachedMeta = NULL;

    auto newBase = (uintptr_t)altBasePtr();

    flushFlash();

    erasePages(newBase, bytes / 2);

    auto metaDst = (MetaEntry *)(newBase + bytes / 2);
    auto newBaseP = (uint8_t *)newBase;
    freeDataPtr = newBaseP + sizeof(FSHeader);

    for (int iter = 0; iter < 2; ++iter) {
        clearBlocked();
        auto offset = sizeof(FSHeader);
        for (auto p = endPtr - 1; p >= metaPtr; p--) {
            MetaEntry m = *p;
            const char *fn = basePtr + m.fnptr;

            if (filter && !filter(fn))
                continue;

            if (checkBlocked(&m) || m.dataptr == 0)
                continue;

            auto fnlen = strlen(fn) + 1;
            auto sz = fnlen + m.datasize();

            if (iter == 0) {
                auto fd = freeDataPtr;
                writeData(fn, fnlen);
                writeData(basePtr + m.dataptr, m.datasize());
                if (freeDataPtr - fd != sz)
                    oops();
            } else {
                m.fnptr = offset;
                m.dataptr = offset + fnlen;
                m._datasize &= ~RAFFS_FOLLOWING_MASK;
                offset += sz;
                writeBytes(--metaDst, &m, sizeof(m));
            }
        }
        if (iter == 0)
            finishWrite();
    }

    clearBlocked();
    flushFlash();

    LOG("GC done: %d free", (int)((intptr_t)metaDst - (intptr_t)dataDst));

    FSHeader hd;
    hd.magic = RAFFS_MAGIC;
    hd.bytes = bytes;
    writeBytes(newBaseP, &hd, sizeof(hd));

    // clear old magic
    hd.magic = 0;
    hd.bytes = 0;
#ifdef SAMD51
    erasePages((uintptr_t)basePtr, flash.pageSize((uintptr_t)basePtr));
#endif
    writeBytes(basePtr, &hd, sizeof(hd));

    flushFlash();

    basePtr = newBaseP;
    endPtr = (MetaEntry *)(newBase + bytes / 2);
    metaPtr = metaDst;

    if ((intptr_t)metaDst - (intptr_t)dataDst <= spaceNeeded + 32) {
        if (filter != NULL && spaceNeeded != 0x7fff0000) {
            LOG("out of space! needed=%d", spaceNeeded);
#ifdef RAFFS_TEST
            oops();
#endif
        }
        return false;
    }

    return true;
}

DirEntry *FS::dirRead() {
    lock();

    if (readDirPtr == NULL) {
        readDirPtr = endPtr - 1;
        clearBlocked();
    }

    while (readDirPtr >= metaPtr) {
        auto m = *readDirPtr--;
        if (checkBlocked(&m) || m.dataptr == 0)
            continue;
        dirEnt.size = m.datasize();
        dirEnt.flags = 0;
        dirEnt.name = (const char *)(basePtr + p->fnptr);
        unlock();
        return &dirEnt;
    }

    readDirPtr = NULL;
    clearBlocked();
    unlock();
    return NULL;
}

uint16_t FS::writeData(const void *data, uint32_t len) {
    writeBytes(freeDataPtr, data, len);
    auto r = freeDataPtr - basePtr;
    freeDataPtr += len;
    return r;
}

void FS::finishWrite() {
    auto nfp = RAFFS_ROUND(freeDataPtr);
    int tailSz = nfp - freeDataPtr;
    uint64_t z = 0;
    if (tailSz) {
        writeData(&z, tailSz);
    } else {
        if (((uint32_t *)nfp)[-1] == M1)
            writeData(&z, 8);
    }
    flushFlash();
}

int FS::readFlashBytes(uintptr_t addr, void *buffer, uint32_t len) {
    lock();
    memcpy(buffer, (void *)addr, len);
    unlock();
    return len;
}

#ifdef RAFFS_TEST
void FS::dump() {}

void FS::debugDump() {
    // dump();
}
#endif