#define __BYTE_QUEUE_CLASS_IMPLEMENT__
#include "byte_queue.h"

#if USE_SERVICE_QUEUE == ENABLED
#undef this
#define this        (*ptThis)

byte_queue_t * queue_init_byte(byte_queue_t *ptObj, void *pBuffer, uint16_t hwItemSize)
{
    class_internal(ptObj, ptThis, byte_queue_t);

    assert(NULL != ptObj);

    if(pBuffer == NULL || hwItemSize == 0) {
        return NULL;
    }

    __protect_queue__ {
        this.pchBuffer = pBuffer;
        this.hwSize = hwItemSize;
        this.hwHead = 0;
        this.hwTail = 0;
        this.hwLength = 0;
        this.hwPeek = this.hwHead;
        this.hwPeekLength = 0;
    }
    return ptObj;
}

bool reset_queue(byte_queue_t *ptObj)
{
    class_internal(ptObj, ptThis, byte_queue_t);

    assert(NULL != ptObj);

    __protect_queue__ {
        this.hwHead = 0;
        this.hwTail = 0;
        this.hwLength = 0;
        this.hwPeek = this.hwHead;
        this.hwPeekLength = 0;
    }
    return true;
}

bool enqueue_byte(byte_queue_t *ptObj, uint8_t chByte)
{
    class_internal(ptObj, ptThis, byte_queue_t);

    assert(NULL != ptObj);

    if(this.hwHead == this.hwTail &&
       0 != this.hwLength ) {
        return false;
    }

    __protect_queue__ {
        this.pchBuffer[this.hwTail++] = chByte;

        if(this.hwTail >= this.hwSize)
        {
            this.hwTail = 0;
        }

        this.hwLength++;
        this.hwPeekLength++;
    }
    return true;
}

int16_t enqueue_bytes(byte_queue_t *ptObj, void *pDate, uint16_t hwLength)
{
    class_internal(ptObj, ptThis, byte_queue_t);

    assert(NULL != ptObj);

    if(this.hwHead == this.hwTail &&
       0 != this.hwLength ) {
        return 0;
    }

    __protect_queue__ {
        uint8_t *pchByte = pDate;

        if(hwLength > (this.hwSize - this.hwLength))
        {
            hwLength = this.hwSize - this.hwLength;
        }

        do
        {
            if(hwLength < (this.hwSize - this.hwTail)) {
                memcpy(&this.pchBuffer[this.hwTail], pchByte, hwLength);
                this.hwTail += hwLength;
                break;
            }

            memcpy(&this.pchBuffer[this.hwTail], &pchByte[0], this.hwSize - this.hwTail);
            memcpy(&this.pchBuffer[0], &pchByte[this.hwSize - this.hwTail], hwLength - (this.hwSize - this.hwTail));
            this.hwTail = hwLength - (this.hwSize - this.hwTail);
        } while(0);

        this.hwLength += hwLength;
        this.hwPeekLength += hwLength;
    }
    return hwLength;
}

bool dequeue_byte(byte_queue_t *ptObj, uint8_t *pchByte)
{
    class_internal(ptObj, ptThis, byte_queue_t);

    assert(NULL != ptObj);

    if(pchByte == NULL) {
        return false;
    }

    if(this.hwHead == this.hwTail &&
       0 == this.hwLength ) {
        return false;
    }

    __protect_queue__ {
        *pchByte = this.pchBuffer[this.hwHead++];

        if(this.hwHead >= this.hwSize)
        {
            this.hwHead = 0;
        }

        this.hwLength--;
        this.hwPeek = this.hwHead;
        this.hwPeekLength = this.hwLength;
    }
    return true;
}

int16_t dequeue_bytes(byte_queue_t *ptObj, void *pDate, uint16_t hwLength)
{
    class_internal(ptObj, ptThis, byte_queue_t);

    assert(NULL != ptObj);

    if(pDate == NULL) {
        return 0;
    }

    if(this.hwHead == this.hwTail &&
       0 == this.hwLength ) {
        return 0;
    }

    uint8_t *pchByte = pDate;
    __protect_queue__ {
        if(hwLength > this.hwLength)
        {
            hwLength = this.hwLength;
        }

        do
        {
            if(hwLength < (this.hwSize - this.hwHead)) {
                memcpy(pchByte, &this.pchBuffer[this.hwHead], hwLength);
                this.hwHead += hwLength;
                break;
            }

            memcpy(&pchByte[0], &this.pchBuffer[this.hwHead], this.hwSize - this.hwHead);
            memcpy(&pchByte[this.hwSize - this.hwHead], &this.pchBuffer[0], hwLength - (this.hwSize - this.hwHead));
            this.hwHead = hwLength - (this.hwSize - this.hwHead);
        } while(0);

        this.hwLength -= hwLength;
        this.hwPeek = this.hwHead;
        this.hwPeekLength = this.hwLength;
    }
    return hwLength;
}

bool is_queue_empty(byte_queue_t *ptObj)
{
    class_internal(ptObj, ptThis, byte_queue_t);

    assert(NULL != ptObj);

    if(this.hwHead == this.hwTail &&
       0 == this.hwLength ) {
        return true;
    }

    return false;
}

int16_t get_queue_count(byte_queue_t *ptObj)
{
    class_internal(ptObj, ptThis, byte_queue_t);

    assert(NULL != ptObj);

    return (this.hwLength);
}

int16_t get_queue_available_count(byte_queue_t *ptObj)
{
    class_internal(ptObj, ptThis, byte_queue_t);

    assert(NULL != ptObj);

    return (this.hwSize - this.hwLength);
}

bool is_peek_empty(byte_queue_t *ptObj)
{
    class_internal(ptObj, ptThis, byte_queue_t);

    assert(NULL != ptObj);

    if(this.hwPeek == this.hwTail &&
       0 == this.hwPeekLength ) {
        return true;
    }

    return false;
}

bool peek_byte_queue(byte_queue_t *ptObj, uint8_t *pchByte)
{
    class_internal(ptObj, ptThis, byte_queue_t);

    assert(NULL != ptObj);

    if(pchByte == NULL) {
        return false;
    }

    if(this.hwPeek == this.hwTail &&
       0 == this.hwPeekLength ) {
        return false;
    }

    __protect_queue__ {
        *pchByte = this.pchBuffer[this.hwPeek++];

        if(this.hwPeek >= this.hwSize)
        {
            this.hwPeek = 0;
        }

        this.hwPeekLength--;
    }
    return true;
}

int16_t peek_bytes_queue(byte_queue_t *ptObj, void *pDate, uint16_t hwLength)
{
    class_internal(ptObj, ptThis, byte_queue_t);

    assert(NULL != ptObj);

    if(pDate == NULL) {
        return 0;
    }

    if(this.hwPeek == this.hwTail &&
       0 == this.hwPeekLength ) {
        return 0;
    }

    __protect_queue__ {
        uint8_t *pchByte = pDate;

        if(hwLength > this.hwPeekLength)
        {
            hwLength = this.hwPeekLength;
        }

        do
        {
            if(hwLength < (this.hwSize - this.hwPeek)) {
                memcpy(pchByte, &this.pchBuffer[this.hwPeek], hwLength);
                this.hwPeek += hwLength;
                break;
            }

            memcpy(&pchByte[0], &this.pchBuffer[this.hwPeek], this.hwSize - this.hwPeek);
            memcpy(&pchByte[this.hwSize - this.hwPeek], &this.pchBuffer[0], hwLength - (this.hwSize - this.hwPeek));
            this.hwPeek = hwLength - (this.hwSize - this.hwPeek);
        } while(0);

        this.hwPeekLength -= hwLength;
    }
    return hwLength;
}

bool reset_peek(byte_queue_t *ptObj)
{
    class_internal(ptObj, ptThis, byte_queue_t);

    assert(NULL != ptObj);

    __protect_queue__ {
        this.hwPeek = this.hwHead;
        this.hwPeekLength = this.hwLength;
    }
    return true;
}

bool get_all_peeked(byte_queue_t *ptObj)
{
    class_internal(ptObj, ptThis, byte_queue_t);

    assert(NULL != ptObj);

    __protect_queue__ {
        this.hwHead = this.hwPeek;
        this.hwLength = this.hwPeekLength;
    }
    return true;
}

uint16_t get_peek_status(byte_queue_t *ptObj)
{
    uint16_t hwCount;
    class_internal(ptObj, ptThis, byte_queue_t);

    assert(NULL != ptObj);

    __protect_queue__ {
        if(this.hwPeek >= this.hwHead)
        {
            hwCount = this.hwPeek - this.hwHead;
        } else
        {
            hwCount = this.hwSize - this.hwHead + this.hwPeek;
        }
    }
    return hwCount;
}

bool restore_peek_status(byte_queue_t *ptObj, uint16_t hwCount)
{
    class_internal(ptObj, ptThis, byte_queue_t);

    assert(NULL != ptObj);

    __protect_queue__ {
        if(this.hwHead + hwCount < this.hwSize)
        {
            this.hwPeek = this.hwHead + hwCount;
        } else
        {
            this.hwPeek =  hwCount - (this.hwSize - this.hwHead);
        }

        this.hwPeekLength = this.hwPeekLength - hwCount;
    }
    return true;
}

#endif
