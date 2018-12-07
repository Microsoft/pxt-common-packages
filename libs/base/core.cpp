#include "pxtbase.h"
#include <limits.h>
#include <stdlib.h>

using namespace std;

#define p10(v) __builtin_powi(10, v)

namespace pxt {

static HandlerBinding *handlerBindings;

HandlerBinding *findBinding(int source, int value) {
    for (auto p = handlerBindings; p; p = p->next) {
        if (p->source == source && p->value == value) {
            return p;
        }
    }
    return 0;
}

void setBinding(int source, int value, Action act) {
    auto curr = findBinding(source, value);
    incr(act);
    if (curr) {
        decr(curr->action);
        curr->action = act;
        return;
    }
    curr = new HandlerBinding();
    curr->next = handlerBindings;
    curr->source = source;
    curr->value = value;
    curr->action = act;
    registerGC(&curr->action);
    handlerBindings = curr;
}

PXT_DEF_STRING(emptyString, "")

static const char emptyBuffer[] __attribute__((aligned(4))) = "@PXT#:\x00\x00\x00";

String mkString(const char *data, int len) {
    if (len < 0)
        len = strlen(data);
    if (len == 0)
        return (String)emptyString;
    String r = new (gcAllocate(sizeof(BoxedString) + len + 1)) BoxedString();
    r->length = len;
    if (data)
        memcpy(r->data, data, len);
    r->data[len] = 0;
    MEMDBG("mkString: len=%d => %p", len, r);
    return r;
}

Buffer mkBuffer(const uint8_t *data, int len) {
    if (len <= 0)
        return (Buffer)emptyBuffer;
    Buffer r = new (gcAllocate(sizeof(BoxedBuffer) + len)) BoxedBuffer();
    r->length = len;
    if (data)
        memcpy(r->data, data, len);
    else
        memset(r->data, 0, len);
    MEMDBG("mkBuffer: len=%d => %p", len, r);
    return r;
}

static unsigned random_value = 0xC0DA1;

void seedRandom(unsigned seed) {
    random_value = seed;
}

unsigned getRandom(unsigned max) {
    unsigned m, result;

    do {
        m = (unsigned)max;
        result = 0;

        do {
            // Cycle the LFSR (Linear Feedback Shift Register).
            // We use an optimal sequence with a period of 2^32-1, as defined by Bruce Schneier here
            // (a true legend in the field!),
            // For those interested, it's documented in his paper:
            // "Pseudo-Random Sequence Generator for 32-Bit CPUs: A fast, machine-independent
            // generator for 32-bit Microprocessors"
            // https://www.schneier.com/paper-pseudorandom-sequence.html
            unsigned r = random_value;

            r = ((((r >> 31) ^ (r >> 6) ^ (r >> 4) ^ (r >> 2) ^ (r >> 1) ^ r) & 1) << 31) |
                (r >> 1);

            random_value = r;

            result = ((result << 1) | (r & 0x00000001));
        } while (m >>= 1);
    } while (result > (unsigned)max);

    return result;
}

PXT_DEF_STRING(sTrue, "true")
PXT_DEF_STRING(sFalse, "false")
PXT_DEF_STRING(sUndefined, "undefined")
PXT_DEF_STRING(sNull, "null")
PXT_DEF_STRING(sObject, "[Object]")
PXT_DEF_STRING(sFunction, "[Function]")
PXT_DEF_STRING(sNaN, "NaN")
PXT_DEF_STRING(sInf, "Infinity")
PXT_DEF_STRING(sMInf, "-Infinity")
} // namespace pxt

#ifndef X86_64

namespace String_ {

//%
String mkEmpty() {
    return mkString("", 0);
}

//%
String fromCharCode(int code) {
    char buf[] = {(char)code, 0};
    return mkString(buf, 1);
}

//%
String charAt(String s, int pos) {
    if (s && 0 <= pos && pos < s->length) {
        return fromCharCode(s->data[pos]);
    } else {
        return mkEmpty();
    }
}

//%
TNumber charCodeAt(String s, int pos) {
    if (s && 0 <= pos && pos < s->length) {
        return fromInt(s->data[pos]);
    } else {
        return TAG_NAN;
    }
}

//%
String concat(String s, String other) {
    if (!s)
        s = (String)sNull;
    if (!other)
        other = (String)sNull;
    if (s->length == 0)
        return (String)incrRC(other);
    if (other->length == 0)
        return (String)incrRC(s);
    String r = mkString(NULL, s->length + other->length);
    memcpy(r->data, s->data, s->length);
    memcpy(r->data + s->length, other->data, other->length);
    return r;
}

int compare(String a, String b) {
    if (a == b)
        return 0;

    int compareResult = strcmp(a->data, b->data);
    if (compareResult < 0)
        return -1;
    else if (compareResult > 0)
        return 1;
    return 0;
}

//%
int length(String s) {
    return s->length;
}

#define isspace(c) ((c) == ' ')

NUMBER mystrtod(const char *p, char **endp) {
    while (isspace(*p))
        p++;
    NUMBER m = 1;
    NUMBER v = 0;
    int dot = 0;
    if (*p == '+')
        p++;
    if (*p == '-') {
        m = -1;
        p++;
    }
    if (*p == '0' && (p[1] | 0x20) == 'x') {
        return m * strtol(p, endp, 16);
    }
    while (*p) {
        int c = *p - '0';
        if (0 <= c && c <= 9) {
            v *= 10;
            v += c;
            if (dot)
                m /= 10;
        } else if (!dot && *p == '.') {
            dot = 1;
        } else if (*p == 'e' || *p == 'E') {
            break;
        } else {
            while (isspace(*p))
                p++;
            if (*p)
                return NAN;
            break;
        }
        p++;
    }

    v *= m;

    if (*p) {
        p++;
        int pw = strtol(p, endp, 10);
        v *= p10(pw);
    } else {
        *endp = (char *)p;
    }

    return v;
}

//%
TNumber toNumber(String s) {
    // JSCHECK
    char *endptr;
    NUMBER v = mystrtod(s->data, &endptr);
    if (endptr != s->data + s->length)
        v = NAN;
    else if (v == 0.0 || v == -0.0)
        v = v;
    else if (!isnormal(v))
        v = NAN;
    return fromDouble(v);
}

//%
String substr(String s, int start, int length) {
    if (length <= 0)
        return mkEmpty();
    if (start < 0)
        start = max(s->length + start, 0);
    length = min(length, s->length - start);
    return mkString(s->data + start, length);
}

//%
int indexOf(String s, String searchString, int start) {
    if (!s || !searchString)
        return -1;
    if (start < 0 || start + searchString->length > s->length)
        return -1;
    const char *match = strstr(((const char *)s->data + start), searchString->data);
    if (NULL == match)
        return -1;
    return match - s->data;
}

//%
int includes(String s, String searchString, int start) {
    return -1 != indexOf(s, searchString, start);
}

} // namespace String_

namespace Boolean_ {
//%
bool bang(int v) {
    return v == 0;
}
} // namespace Boolean_

namespace pxt {

// ES5 9.5, 9.6
unsigned toUInt(TNumber v) {
    if (isNumber(v))
        return numValue(v);
    if (isSpecial(v)) {
        if ((intptr_t)v >> 6)
            return 1;
        else
            return 0;
    }
    if (!v)
        return 0;

    NUMBER num = toDouble(v);
    if (!isnormal(num))
        return 0;
#ifdef PXT_USE_FLOAT
    float rem = fmodf(truncf(num), 4294967296.0);
#else
    double rem = fmod(trunc(num), 4294967296.0);
#endif
    if (rem < 0.0)
        rem += 4294967296.0;
    return (unsigned)rem;
}
int toInt(TNumber v) {
    return (int)toUInt(v);
}

NUMBER toDouble(TNumber v) {
    if (v == TAG_NAN || v == TAG_UNDEFINED)
        return NAN;
    if (isTagged(v))
        return toInt(v);

    ValType t = valType(v);
    if (t == ValType::Number) {
        BoxedNumber *p = (BoxedNumber *)v;
        return p->num;
    } else if (t == ValType::String) {
        // TODO avoid allocation
        auto tmp = String_::toNumber((String)v);
        auto r = toDouble(tmp);
        decr(tmp);
        return r;
    } else {
        return NAN;
    }
}

float toFloat(TNumber v) {
    // TODO optimize?
    return (float)toDouble(v);
}

#if !defined(PXT_HARD_FLOAT) && !defined(PXT_USE_FLOAT)
union NumberConv {
    double v;
    struct {
        uint32_t word0;
        uint32_t word1;
    };
};

static inline TValue doubleToInt(double x) {
    NumberConv cnv;
    cnv.v = x;

    if (cnv.word1 == 0 && cnv.word0 == 0)
        return TAG_NUMBER(0);

    auto ex = (int)((cnv.word1 << 1) >> 21) - 1023;

    // DMESG("v=%d/1000 %p %p %d", (int)(x * 1000), cnv.word0, cnv.word1, ex);

    if (ex < 0 || ex > 29) {
        // the 'MININT' case
        if (ex == 30 && cnv.word0 == 0 && cnv.word1 == 0xC1D00000)
            return (TValue)(0x80000001);
        return NULL;
    }

    int32_t r;

    if (ex <= 20) {
        if (cnv.word0)
            return TAG_UNDEFINED;
        if (cnv.word1 << (ex + 12))
            return TAG_UNDEFINED;
        r = ((cnv.word1 << 11) | 0x80000000) >> (20 - ex + 11);
    } else {
        if (cnv.word0 << (ex - 20))
            return TAG_UNDEFINED;
        r = ((cnv.word1 << 11) | 0x80000000) >> (20 - ex + 11);
        r |= cnv.word0 >> (32 - (ex - 20));
    }

    if (cnv.word1 >> 31)
        return TAG_NUMBER(-r);
    else
        return TAG_NUMBER(r);
}
#else
static inline TValue doubleToInt(NUMBER r) {
    int ri = ((int)r) << 1;
    if ((ri >> 1) == r)
        return (TNumber)(ri | 1);
    return TAG_UNDEFINED;
}
#endif

TNumber fromDouble(NUMBER r) {
#ifndef PXT_BOX_DEBUG
    auto i = doubleToInt(r);
    if (i)
        return i;
#endif
    if (isnan(r))
        return TAG_NAN;
    BoxedNumber *p = NEW_GC(BoxedNumber);
    p->num = r;
    MEMDBG("mkNum: %d/1000 => %p", (int)(r * 1000), p);
    return (TNumber)p;
}

TNumber fromFloat(float r) {
    // TODO optimize
    return fromDouble(r);
}

TNumber fromInt(int v) {
    if (canBeTagged(v))
        return TAG_NUMBER(v);
    return fromDouble(v);
}

TNumber fromUInt(unsigned v) {
#ifndef PXT_BOX_DEBUG
    if (v <= 0x3fffffff)
        return TAG_NUMBER(v);
#endif
    return fromDouble(v);
}

TValue fromBool(bool v) {
    if (v)
        return TAG_TRUE;
    else
        return TAG_FALSE;
}

TNumber eqFixup(TNumber v) {
    if (v == TAG_NULL)
        return TAG_UNDEFINED;
    if (v == TAG_TRUE)
        return TAG_NUMBER(1);
    if (v == TAG_FALSE)
        return TAG_NUMBER(0);
    return v;
}

static inline bool eq_core(TValue a, TValue b, ValType ta) {
#ifndef PXT_BOX_DEBUG
    int aa = (int)a;
    int bb = (int)b;

    // if at least one of the values is tagged, they are not equal
    if ((aa | bb) & 3)
        return false;
#endif

    if (ta == ValType::String)
        return String_::compare((String)a, (String)b) == 0;
    else if (ta == ValType::Number)
        return toDouble(a) == toDouble(b);
    else
        return a == b;
}

bool eqq_bool(TValue a, TValue b) {
    if (a == TAG_NAN || b == TAG_NAN)
        return false;

    if (a == b)
        return true;

    if (bothNumbers(a, b))
        return false;

    ValType ta = valType(a);
    ValType tb = valType(b);

    if (ta != tb)
        return false;

    return eq_core(a, b, ta);
}

bool eq_bool(TValue a, TValue b) {
    if (a == TAG_NAN || b == TAG_NAN)
        return false;

    if (eqFixup(a) == eqFixup(b))
        return true;

    if (bothNumbers(a, b))
        return false;

    ValType ta = valType(a);
    ValType tb = valType(b);

    if ((ta == ValType::String && tb == ValType::Number) ||
        (tb == ValType::String && ta == ValType::Number))
        return toDouble(a) == toDouble(b);

    if (ta == ValType::Boolean) {
        a = eqFixup(a);
        ta = ValType::Number;
    }
    if (tb == ValType::Boolean) {
        b = eqFixup(b);
        tb = ValType::Number;
    }

    if (ta != tb)
        return false;

    return eq_core(a, b, ta);
}

// TODO move to assembly
//%
bool switch_eq(TValue a, TValue b) {
    if (eq_bool(a, b)) {
        decr(b);
        return true;
    }
    return false;
}

} // namespace pxt

#define NUMOP(op) return fromDouble(toDouble(a) op toDouble(b));
#define BITOP(op) return fromInt(toInt(a) op toInt(b));
namespace numops {

int toBool(TValue v) {
    if (isTagged(v)) {
        if (v == TAG_FALSE || v == TAG_UNDEFINED || v == TAG_NAN || v == TAG_NULL ||
            v == TAG_NUMBER(0))
            return 0;
        else
            return 1;
    }

    ValType t = valType(v);
    if (t == ValType::String) {
        String s = (String)v;
        if (s->length == 0)
            return 0;
    } else if (t == ValType::Number) {
        auto x = toDouble(v);
        if (isnan(x) || x == 0.0 || x == -0.0)
            return 0;
        else
            return 1;
    }

    return 1;
}

int toBoolDecr(TValue v) {
    if (v == TAG_TRUE)
        return 1;
    if (v == TAG_FALSE)
        return 0;
    int r = toBool(v);
    decr(v);
    return r;
}

// TODO
// The integer, non-overflow case for add/sub/bit opts is handled in assembly

//%
TNumber adds(TNumber a, TNumber b){NUMOP(+)}

//%
TNumber subs(TNumber a, TNumber b){NUMOP(-)}

//%
TNumber muls(TNumber a, TNumber b) {
    if (bothNumbers(a, b)) {
        int aa = (int)a;
        int bb = (int)b;
        // if both operands fit 15 bits, the result will not overflow int
        if ((aa >> 15 == 0 || aa >> 15 == -1) && (bb >> 15 == 0 || bb >> 15 == -1)) {
            // it may overflow 31 bit int though - use fromInt to convert properly
            return fromInt((aa >> 1) * (bb >> 1));
        }
    }
    NUMOP(*)
}

//%
TNumber div(TNumber a, TNumber b){NUMOP(/)}

//%
TNumber mod(TNumber a, TNumber b) {
    if (isNumber(a) && isNumber(b) && numValue(b))
        BITOP(%)
    return fromDouble(fmod(toDouble(a), toDouble(b)));
}

//%
TNumber lsls(TNumber a, TNumber b){BITOP(<<)}

//%
TNumber lsrs(TNumber a, TNumber b) {
    return fromUInt(toUInt(a) >> toUInt(b));
}

//%
TNumber asrs(TNumber a, TNumber b){BITOP(>>)}

//%
TNumber eors(TNumber a, TNumber b){BITOP (^)}

//%
TNumber orrs(TNumber a, TNumber b){BITOP(|)}

//%
TNumber bnot(TNumber a) {
    return fromInt(~toInt(a));
}

//%
TNumber ands(TNumber a, TNumber b) {
    BITOP(&)
}

#define CMPOP_RAW(op, t, f)                                                                        \
    if (bothNumbers(a, b))                                                                         \
        return (int)a op((int)b) ? t : f;                                                          \
    int cmp = valCompare(a, b);                                                                    \
    return cmp != -2 && cmp op 0 ? t : f;

#define CMPOP(op) CMPOP_RAW(op, TAG_TRUE, TAG_FALSE)

// 7.2.13 Abstract Relational Comparison
static int valCompare(TValue a, TValue b) {
    if (a == TAG_NAN || b == TAG_NAN)
        return -2;

    ValType ta = valType(a);
    ValType tb = valType(b);

    if (ta == ValType::String && tb == ValType::String)
        return String_::compare((String)a, (String)b);

    if (a == b)
        return 0;

    auto da = toDouble(a);
    auto db = toDouble(b);

    if (isnan(da) || isnan(db))
        return -2;

    if (da < db)
        return -1;
    else if (da > db)
        return 1;
    else
        return 0;
}

//%
bool lt_bool(TNumber a, TNumber b){CMPOP_RAW(<, true, false)}

//%
TNumber le(TNumber a, TNumber b){CMPOP(<=)}

//%
TNumber lt(TNumber a, TNumber b){CMPOP(<)}

//%
TNumber ge(TNumber a, TNumber b){CMPOP(>=)}

//%
TNumber gt(TNumber a, TNumber b){CMPOP(>)}

//%
TNumber eq(TNumber a, TNumber b) {
    return pxt::eq_bool(a, b) ? TAG_TRUE : TAG_FALSE;
}

//%
TNumber neq(TNumber a, TNumber b) {
    return !pxt::eq_bool(a, b) ? TAG_TRUE : TAG_FALSE;
}

//%
TNumber eqq(TNumber a, TNumber b) {
    return pxt::eqq_bool(a, b) ? TAG_TRUE : TAG_FALSE;
}

//%
TNumber neqq(TNumber a, TNumber b) {
    return !pxt::eqq_bool(a, b) ? TAG_TRUE : TAG_FALSE;
}

void mycvt(NUMBER d, char *buf) {
    if (d < 0) {
        *buf++ = '-';
        d = -d;
    }

    if (!d) {
        *buf++ = '0';
        *buf++ = 0;
        return;
    }

    int pw = (int)log10(d);
    int e = 1;
    int beforeDot = 1;

    if (0.000001 <= d && d < 1e21) {
        if (pw > 0) {
            d /= p10(pw);
            beforeDot = 1 + pw;
        }
    } else {
        d /= p10(pw);
        e = pw;
    }

    int sig = 0;
    while (sig < 17 || beforeDot > 0) {
        // printf("%f sig=%d bd=%d\n", d, sig, beforeDot);
        int c = (int)d;
        *buf++ = '0' + c;
        d = (d - c) * 10;
        if (--beforeDot == 0)
            *buf++ = '.';
        if (sig || c)
            sig++;
    }

    buf--;
    while (*buf == '0')
        buf--;
    if (*buf == '.')
        buf--;
    buf++;

    if (e != 1) {
        *buf++ = 'e';
        itoa(e, buf);
    } else {
        *buf = 0;
    }
}

#if 0
//%
TValue floatAsInt(TValue x) {
    return doubleToInt(toDouble(x));
}

//% shim=numops::floatAsInt
function floatAsInt(v: number): number { return 0 }

function testInt(i: number) {
    if (floatAsInt(i) != i)
        control.panic(101)
    if (floatAsInt(i + 0.5) != null)
        control.panic(102)
    if (floatAsInt(i + 0.00001) != null)
        control.panic(103)
}

function testFloat(i: number) {
    if (floatAsInt(i) != null)
        control.panic(104)
}

function testFloatAsInt() {
    for (let i = 0; i < 0xffff; ++i) {
        testInt(i)
        testInt(-i)
        testInt(i * 10000)
        testInt(i << 12)
        testInt(i + 0x3fff0001)
        testInt(-i - 0x3fff0002)
        testFloat(i + 0x3fffffff + 1)
        testFloat((i + 10000) * 1000000)
    }   
}
#endif

String toString(TValue v) {
    ValType t = valType(v);

    if (t == ValType::String) {
        return (String)(void *)incr(v);
    } else if (t == ValType::Number) {
        char buf[64];

        if (isNumber(v)) {
            itoa(numValue(v), buf);
            return mkString(buf);
        }

        if (v == TAG_NAN)
            return (String)(void *)sNaN;

        auto x = toDouble(v);

#ifdef PXT_BOX_DEBUG
        if (x == (int)x) {
            itoa((int)x, buf);
            return mkString(buf);
        }
#endif

        if (isinf(x)) {
            if (x < 0)
                return (String)(void *)sMInf;
            else
                return (String)(void *)sInf;
        } else if (isnan(x)) {
            return (String)(void *)sNaN;
        }
        mycvt(x, buf);

        return mkString(buf);
    } else if (t == ValType::Function) {
        return (String)(void *)sFunction;
    } else {
        if (v == TAG_UNDEFINED)
            return (String)(void *)sUndefined;
        else if (v == TAG_FALSE)
            return (String)(void *)sFalse;
        else if (v == TAG_NAN)
            return (String)(void *)sNaN;
        else if (v == TAG_TRUE)
            return (String)(void *)sTrue;
        else if (v == TAG_NULL)
            return (String)(void *)sNull;
        return (String)(void *)sObject;
    }
}

} // namespace numops

namespace Math_ {
//%
TNumber pow(TNumber x, TNumber y) {
#ifdef PXT_POWI
    // regular pow() from math.h is 4k of code
    return fromDouble(__builtin_powi(toDouble(x), toInt(y)));
#else
    return fromDouble(::pow(toDouble(x), toDouble(y)));
#endif
}

//%
TNumber atan2(TNumber y, TNumber x) {
    return fromDouble(::atan2(toDouble(y), toDouble(x)));
}

NUMBER randomDouble() {
    return getRandom(UINT_MAX) / ((NUMBER)UINT_MAX + 1) +
           getRandom(0xffffff) / ((NUMBER)UINT_MAX * 0xffffff);
}

//%
TNumber random() {
    return fromDouble(randomDouble());
}

//%
TNumber randomRange(TNumber min, TNumber max) {
    if (isNumber(min) && isNumber(max)) {
        int mini = numValue(min);
        int maxi = numValue(max);
        if (mini > maxi) {
            int temp = mini;
            mini = maxi;
            maxi = temp;
        }
        if (maxi == mini)
            return fromInt(mini);
        else
            return fromInt(mini + getRandom(maxi - mini));
    } else {
        auto mind = toDouble(min);
        auto maxd = toDouble(max);
        if (mind > maxd) {
            auto temp = mind;
            mind = maxd;
            maxd = temp;
        }
        if (maxd == mind)
            return fromDouble(mind);
        else {
            return fromDouble(mind + randomDouble() * (maxd - mind));
        }
    }
}

#define SINGLE(op) return fromDouble(::op(toDouble(x)));

//%
TNumber log(TNumber x){SINGLE(log)}

//%
TNumber log10(TNumber x){SINGLE(log10)}

//%
TNumber tan(TNumber x){SINGLE(tan)}

//%
TNumber sin(TNumber x){SINGLE(sin)}

//%
TNumber cos(TNumber x){SINGLE(cos)}

//%
TNumber atan(TNumber x){SINGLE(atan)}

//%
TNumber asin(TNumber x){SINGLE(asin)}

//%
TNumber acos(TNumber x){SINGLE(acos)}

//%
TNumber sqrt(TNumber x){SINGLE(sqrt)}

//%
TNumber floor(TNumber x){SINGLE(floor)}

//%
TNumber ceil(TNumber x){SINGLE(ceil)}

//%
TNumber trunc(TNumber x){SINGLE(trunc)}

//%
TNumber round(TNumber x) {
    // In C++, round(-1.5) == -2, while in JS, round(-1.5) == -1. Align to the JS convention for
    // consistency between simulator and device. The following does rounding with ties (x.5) going
    // towards positive infinity.
    return fromDouble(::floor(toDouble(x) + 0.5));
}

//%
int imul(int x, int y) {
    return x * y;
}

//%
int idiv(int x, int y) {
    return x / y;
}
} // namespace Math_

namespace Array_ {
RefCollection *mk() {
    auto r = NEW_GC(RefCollection);
    MEMDBG("mkColl: => %p", r);
    return r;
}
int length(RefCollection *c) {
    return c->length();
}
void setLength(RefCollection *c, int newLength) {
    c->setLength(newLength);
}
void push(RefCollection *c, TValue x) {
    c->head.push(x);
}
TValue pop(RefCollection *c) {
    return c->head.pop();
}
TValue getAt(RefCollection *c, int x) {
    return c->head.get(x);
}
void setAt(RefCollection *c, int x, TValue y) {
    c->head.set(x, y);
}
TValue removeAt(RefCollection *c, int x) {
    return c->head.remove(x);
}
void insertAt(RefCollection *c, int x, TValue value) {
    c->head.insert(x, value);
}
int indexOf(RefCollection *c, TValue x, int start) {
    auto data = c->head.getData();
    auto len = c->head.getLength();
    for (unsigned i = 0; i < len; i++) {
        if (pxt::eq_bool(data[i], x)) {
            return (int)i;
        }
    }
    return -1;
}
bool removeElement(RefCollection *c, TValue x) {
    int idx = indexOf(c, x, 0);
    if (idx >= 0) {
        decr(removeAt(c, idx));
        return 1;
    }
    return 0;
}
} // namespace Array_

namespace pxt {
//%
void *ptrOfLiteral(int offset);

//%
unsigned programSize() {
    return bytecode[17] * 8;
}

//%
int getConfig(int key, int defl) {
    int *cfgData;

#ifdef PXT_BOOTLOADER_CFG_ADDR
    cfgData = *(int **)(PXT_BOOTLOADER_CFG_ADDR);
#ifdef PXT_BOOTLOADER_CFG_MAGIC
    cfgData++;
    if ((void *)0x200 <= cfgData && cfgData < (void *)PXT_BOOTLOADER_CFG_ADDR &&
        cfgData[-1] == (int)PXT_BOOTLOADER_CFG_MAGIC)
#endif
        for (int i = 0;; i += 2) {
            if (cfgData[i] == key)
                return cfgData[i + 1];
            if (cfgData[i] == 0)
                break;
        }
#endif

    cfgData = *(int **)&bytecode[18];
    for (int i = 0;; i += 2) {
        if (cfgData[i] == key)
            return cfgData[i + 1];
        if (cfgData[i] == 0)
            return defl;
    }
}

} // namespace pxt

namespace pxtrt {
//%
TValue ldlocRef(RefRefLocal *r) {
    TValue tmp = r->v;
    incr(tmp);
    return tmp;
}

//%
void stlocRef(RefRefLocal *r, TValue v) {
    decr(r->v);
    r->v = v;
}

//%
RefRefLocal *mklocRef() {
    auto r = NEW_GC(RefRefLocal);
    MEMDBG("mklocRef: => %p", r);
    return r;
}

// Store a captured local in a closure. It returns the action, so it can be chained.
//%
RefAction *stclo(RefAction *a, int idx, TValue v) {
    // DBG("STCLO "); a->print(); DBG("@%d = %p\n", idx, (void*)v);
    a->stCore(idx, v);
    return a;
}

//%
void panic(int code) {
    target_panic(code);
}

//%
String emptyToNull(String s) {
    if (!s || s->length == 0)
        return NULL;
    return s;
}

//%
int ptrToBool(TValue p) {
    if (p) {
        decr(p);
        return 1;
    } else {
        return 0;
    }
}

//%
RefMap *mkMap() {
    auto r = NEW_GC(RefMap);
    MEMDBG("mkMap: => %p", r);
    return r;
}

//%
TValue mapGetByString(RefMap *map, String key) {
    int i = map->findIdx(key);
    if (i < 0) {
        return 0;
    }
    TValue r = incr(map->values.get(i));
    return r;
}

//%
int lookupMapKey(String key) {
    auto arr = *(uintptr_t **)&bytecode[22];
    auto len = *arr++;
    auto ikey = (uintptr_t)key;
    auto l = 0U;
    auto r = len - 1;
    if (arr[0] <= ikey && ikey <= arr[len - 1]) {
        while (l <= r) {
            auto m = (l + r) >> 1;
            if (arr[m] == ikey)
                return m;
            else if (arr[m] < ikey)
                l = m + 1;
            else
                r = m - 1;
        }
    } else {
        while (l <= r) {
            auto m = (l + r) >> 1;
            auto cmp = String_::compare((String)arr[m], key);
            if (cmp == 0)
                return m;
            else if (cmp < 0)
                l = m + 1;
            else
                r = m - 1;
        }
    }
    return 0;
}

//%
TValue mapGet(RefMap *map, unsigned key) {
    auto arr = *(String **)&bytecode[22];
    auto r = mapGetByString(map, arr[key + 1]);
    map->unref();
    return r;
}

//%
void mapSetByString(RefMap *map, String key, TValue val) {
    int i = map->findIdx(key);
    if (i < 0) {
        incrRC(key);
        map->keys.push((TValue)key);
        map->values.push(val);
    } else {
        map->values.set(i, val);
    }
    incr(val);
}

//%
void mapSet(RefMap *map, unsigned key, TValue val) {
    auto arr = *(String **)&bytecode[22];
    mapSetByString(map, arr[key + 1], val);
    decr(val);
    map->unref();
}

//
// Debugger
//

// This is only to be called once at the beginning of lambda function
//%
void *getGlobalsPtr() {
#ifdef DEVICE_GROUP_ID_USER
    fiber_set_group(DEVICE_GROUP_ID_USER);
#endif

    return globals;
}

//%
void runtimeWarning(String s) {
    // noop for now
}
} // namespace pxtrt
#endif

namespace pxt {

//%
ValType valType(TValue v) {
    if (isTagged(v)) {
        if (!v)
            return ValType::Undefined;

        if (isNumber(v) || v == TAG_NAN)
            return ValType::Number;
        if (v == TAG_TRUE || v == TAG_FALSE)
            return ValType::Boolean;
        else if (v == TAG_NULL)
            return ValType::Object;
        else {
            oops(1);
            return ValType::Object;
        }
    } else {
        auto vt = getVTable((RefObject *)v);
        if (vt->magic == VTABLE_MAGIC)
            return vt->objectType;
        else
            return ValType::Object;
    }
}

PXT_DEF_STRING(sObjectTp, "object")
PXT_DEF_STRING(sBooleanTp, "boolean")
PXT_DEF_STRING(sStringTp, "string")
PXT_DEF_STRING(sNumberTp, "number")
PXT_DEF_STRING(sFunctionTp, "function")
PXT_DEF_STRING(sUndefinedTp, "undefined")

//%
String typeOf(TValue v) {
    switch (valType(v)) {
    case ValType::Undefined:
        return (String)sUndefinedTp;
    case ValType::Boolean:
        return (String)sBooleanTp;
    case ValType::Number:
        return (String)sNumberTp;
    case ValType::String:
        return (String)sStringTp;
    case ValType::Object:
        return (String)sObjectTp;
    case ValType::Function:
        return (String)sFunctionTp;
    default:
        oops(2);
        return 0;
    }
}

// Maybe in future we will want separate print methods; for now ignore
void anyPrint(TValue v) {
    if (valType(v) == ValType::Object) {
        if (isRefCounted(v)) {
            auto o = (RefObject *)v;
            auto vt = getVTable(o);
            auto meth = ((RefObjectMethod)vt->methods[1]);
            if ((void *)meth == (void *)&anyPrint)
                DMESG("[RefObject refs=%d vt=%p cl=%d sz=%d]", REFCNT(o), o->vtable, vt->classNo,
                      vt->numbytes);
            else
                meth(o);
        } else {
            DMESG("[Native %p]", v);
        }
    } else {
#ifndef X86_64
        String s = numops::toString(v);
        DMESG("[%s %p = %s]", pxt::typeOf(v)->data, v, s->data);
        decr((TValue)s);
#endif
    }
}

static void dtorDoNothing() {}

#ifdef PXT_GC
#define PRIM_VTABLE(name, objectTp, tp, szexpr)                                                    \
    static uint32_t name##_size(tp *p) { return ((sizeof(tp) + szexpr) + 3) >> 2; }                \
    DEF_VTABLE(name##_vt, tp, objectTp, (void *)&dtorDoNothing, (void *)&anyPrint, 0,              \
               (void *)&name##_size)
#else
#define PRIM_VTABLE(name, objectTp, tp, szexpr)                                                    \
    DEF_VTABLE(name##_vt, tp, objectTp, (void *)&dtorDoNothing, (void *)&anyPrint)
#endif

PRIM_VTABLE(string, ValType::String, BoxedString, p->length + 1)
PRIM_VTABLE(number, ValType::Number, BoxedNumber, 0)
PRIM_VTABLE(buffer, ValType::Object, BoxedBuffer, p->length)
// PRIM_VTABLE(action, ValType::Function, RefAction, )

void failedCast(TValue v) {
    DMESG("failed type check for %p", v);
    auto vt = getAnyVTable(v);
    if (vt) {
        DMESG("VT %p - objtype %d classNo %d", vt, vt->objectType, vt->classNo);
    }

    int code;
    if (v == TAG_NULL)
        code = PANIC_CAST_FROM_NULL;
    else
        code = PANIC_CAST_FIRST + (int)valType(v);
    target_panic(code);
}

void missingProperty(TValue v) {
    DMESG("missing property on %p", v);
    target_panic(PANIC_MISSING_PROPERTY);
}

#ifdef PXT_PROFILE
struct PerfCounter *perfCounters;

struct PerfCounterInfo {
    uint32_t numPerfCounters;
    char *perfCounterNames[0];
};

#define PERF_INFO ((PerfCounterInfo *)(((uintptr_t *)bytecode)[13]))

void initPerfCounters() {
    auto n = PERF_INFO->numPerfCounters;
    perfCounters = new PerfCounter[n];
    memset(perfCounters, 0, n * sizeof(PerfCounter));
}

void dumpPerfCounters() {
    auto info = PERF_INFO;
    DMESG("calls,us,name");
    for (uint32_t i = 0; i < info->numPerfCounters; ++i) {
        auto c = &perfCounters[i];
        DMESG("%d,%d,%s", c->numstops, c->value, info->perfCounterNames[i]);
    }
}

void startPerfCounter(PerfCounters n) {
    auto c = &perfCounters[(uint32_t)n];
    if (c->start)
        oops(50);
    c->start = PERF_NOW();
}

void stopPerfCounter(PerfCounters n) {
    auto c = &perfCounters[(uint32_t)n];
    if (!c->start)
        oops(51);
    c->value += PERF_NOW() - c->start;
    c->start = 0;
    c->numstops++;
}
#endif

} // namespace pxt
