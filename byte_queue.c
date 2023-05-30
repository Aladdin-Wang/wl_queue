/****************************************************************************
*  Copyright 2022 KK (weixin:Aladdin_KK)                                    *
*                                                                           *
*  Licensed under the Apache License, Version 2.0 (the "License");          *
*  you may not use this file except in compliance with the License.         *
*  You may obtain a copy of the License at                                  *
*                                                                           *
*     http://www.apache.org/licenses/LICENSE-2.0                            *
*                                                                           *
*  Unless required by applicable law or agreed to in writing, software      *
*  distributed under the License is distributed on an "AS IS" BASIS,        *
*  WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied. *
*  See the License for the specific language governing permissions and      *
*  limitations under the License.                                           *
*                                                                           *
****************************************************************************/

#define __BYTE_QUEUE_CLASS_IMPLEMENT__
#include "byte_queue.h"

#if USE_SERVICE_QUEUE == ENABLED
#undef this
#define this        (*ptThis)

byte_queue_t * queue_init_byte(byte_queue_t *ptObj, void *pBuffer, uint16_t hwItemSize,bool bIsCover)
{
    assert(NULL != ptObj);
    /* initialise "this" (i.e. ptThis) to access class members */
    class_internal(ptObj, ptThis, byte_queue_t);

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
        this.bIsCover = bIsCover;
    }
    return ptObj;
}

bool reset_queue(byte_queue_t *ptObj)
{
    assert(NULL != ptObj);
    /* initialise "this" (i.e. ptThis) to access class members */
    class_internal(ptObj, ptThis, byte_queue_t);

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
    assert(NULL != ptObj);
    /* initialise "this" (i.e. ptThis) to access class members */
    class_internal(ptObj, ptThis, byte_queue_t);
    bool  bResult = false;
    __protect_queue__ {
        if(this.hwHead == this.hwTail &&
           0 != this.hwLength ){
           if(this.bIsCover == false){
                /* queue is full */
                continue;
            }else{
               /*  overwrite */
                this.hwHead++;
                if(this.hwHead >= this.hwSize){
                    this.hwHead = 0;
                }
                this.hwLength--;
                this.hwPeek = this.hwHead;
                this.hwPeekLength = this.hwLength;
            }
        }
        this.pchBuffer[this.hwTail++] = chByte;
        if(this.hwTail >= this.hwSize){
            this.hwTail = 0;
        }
        this.hwLength++;
        this.hwPeekLength++;
        bResult = true;
    }
    return bResult;
}

uint16_t enqueue_bytes(byte_queue_t *ptObj, void *pDate, uint16_t hwLength)
{
    assert(NULL != ptObj);
    assert(NULL != pDate);
    /* initialise "this" (i.e. ptThis) to access class members */
    class_internal(ptObj, ptThis, byte_queue_t);

    uint8_t *pchByte = pDate;
    __protect_queue__ {
        if(hwLength > this.hwSize){
            hwLength = this.hwSize;
        }
        if(this.hwHead == this.hwTail &&
           0 != this.hwLength ){
            if(this.bIsCover == false){
                /* queue is full */
                hwLength = 0;
                continue;
            }else{
                 /* overwrite */
                if(hwLength < (this.hwSize - this.hwHead)) {
                    this.hwHead += hwLength;
                }else{
                    this.hwHead = hwLength - (this.hwSize - this.hwHead);
                } 
                this.hwLength -= hwLength;
                this.hwPeek = this.hwHead;
                this.hwPeekLength = this.hwLength;                                  
            }
        }
        if(hwLength > (this.hwSize - this.hwLength)){
            if(this.bIsCover == false){
                /* drop some data */
                hwLength = this.hwSize - this.hwLength;
            }else{
                /* overwrite some data */ 
                uint16_t hwOverLength = hwLength - ((this.hwSize - this.hwLength));
                if(hwOverLength < (this.hwSize - this.hwHead)) {
                    this.hwHead += hwOverLength;
                }else{
                    this.hwHead = hwLength - (this.hwSize - this.hwHead);
                } 
                this.hwLength -= hwOverLength;
                this.hwPeek = this.hwHead;
                this.hwPeekLength = this.hwLength;                                         
            }
        }

        do{
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
    assert(NULL != ptObj);
    assert(NULL != pchByte);
    /* initialise "this" (i.e. ptThis) to access class members */
    class_internal(ptObj, ptThis, byte_queue_t);
    bool  bResult = false;
    __protect_queue__ {

        if(this.hwHead == this.hwTail &&
           0 == this.hwLength ){
            /* queue is empty */
            continue;
        }

        *pchByte = this.pchBuffer[this.hwHead++];

        if(this.hwHead >= this.hwSize){
            this.hwHead = 0;
        }

        this.hwLength--;
        this.hwPeek = this.hwHead;
        this.hwPeekLength = this.hwLength;
        bResult = true;
    }
    return bResult;
}

uint16_t dequeue_bytes(byte_queue_t *ptObj, void *pDate, uint16_t hwLength)
{
    assert(NULL != ptObj);
    assert(NULL != pDate);

    /* initialise "this" (i.e. ptThis) to access class members */
    class_internal(ptObj, ptThis, byte_queue_t);

    uint8_t *pchByte = pDate;
    __protect_queue__ {

        if(this.hwHead == this.hwTail &&
           0 == this.hwLength ){
            /* queue is empty */
            hwLength = 0;
            continue;
        }

        if(hwLength > this.hwLength){
            /* less data */
            hwLength = this.hwLength;
        }

        do{
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
    assert(NULL != ptObj);
    /* initialise "this" (i.e. ptThis) to access class members */
    class_internal(ptObj, ptThis, byte_queue_t);

    if(this.hwHead == this.hwTail &&
       0 == this.hwLength ) {
        return true;
    }

    return false;
}

uint16_t get_queue_count(byte_queue_t *ptObj)
{
    assert(NULL != ptObj);
    /* initialise "this" (i.e. ptThis) to access class members */
    class_internal(ptObj, ptThis, byte_queue_t);

    return (this.hwLength);
}

uint16_t get_queue_available_count(byte_queue_t *ptObj)
{
    assert(NULL != ptObj);
    /* initialise "this" (i.e. ptThis) to access class members */
    class_internal(ptObj, ptThis, byte_queue_t);

    return (this.hwSize - this.hwLength);
}

bool is_peek_empty(byte_queue_t *ptObj)
{
    assert(NULL != ptObj);
    /* initialise "this" (i.e. ptThis) to access class members */
    class_internal(ptObj, ptThis, byte_queue_t);

    if(this.hwPeek == this.hwTail &&
       0 == this.hwPeekLength ) {
        return true;
    }

    return false;
}

bool peek_byte_queue(byte_queue_t *ptObj, uint8_t *pchByte)
{
    assert(NULL != ptObj);
    assert(NULL != pchByte);

    /* initialise "this" (i.e. ptThis) to access class members */
    class_internal(ptObj, ptThis, byte_queue_t);
    bool  bResult = false;
    __protect_queue__ {

        if(this.hwPeek == this.hwTail &&
           0 == this.hwPeekLength ){
            /* empty */
            continue;
        }

        *pchByte = this.pchBuffer[this.hwPeek++];

        if(this.hwPeek >= this.hwSize){
            this.hwPeek = 0;
        }

        this.hwPeekLength--;
        bResult = true;
    }
    return bResult;
}

uint16_t peek_bytes_queue(byte_queue_t *ptObj, void *pDate, uint16_t hwLength)
{
    assert(NULL != ptObj);
    assert(NULL != pDate);

    /* initialise "this" (i.e. ptThis) to access class members */
    class_internal(ptObj, ptThis, byte_queue_t);

    uint8_t *pchByte = pDate;

    __protect_queue__ {
        if(this.hwPeek == this.hwTail &&
           0 == this.hwPeekLength ){
            /* empty */
            hwLength = 0;
            continue;
        }

        if(hwLength > this.hwPeekLength){
            /* less data */
            hwLength = this.hwPeekLength;
        }

        do{
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
    assert(NULL != ptObj);
    /* initialise "this" (i.e. ptThis) to access class members */
    class_internal(ptObj, ptThis, byte_queue_t);

    __protect_queue__ {
        this.hwPeek = this.hwHead;
        this.hwPeekLength = this.hwLength;
    }
    return true;
}

bool get_all_peeked(byte_queue_t *ptObj)
{
    assert(NULL != ptObj);
    /* initialise "this" (i.e. ptThis) to access class members */
    class_internal(ptObj, ptThis, byte_queue_t);

    __protect_queue__ {
        this.hwHead = this.hwPeek;
        this.hwLength = this.hwPeekLength;
    }
    return true;
}

uint16_t get_peek_status(byte_queue_t *ptObj)
{
    assert(NULL != ptObj);
    /* initialise "this" (i.e. ptThis) to access class members */
    class_internal(ptObj, ptThis, byte_queue_t);

    uint16_t hwCount;

    __protect_queue__ {
        if(this.hwPeek >= this.hwHead){
            hwCount = this.hwPeek - this.hwHead;
        } else{
            hwCount = this.hwSize - this.hwHead + this.hwPeek;
        }
    }
    return hwCount;
}

bool restore_peek_status(byte_queue_t *ptObj, uint16_t hwCount)
{
    assert(NULL != ptObj);
    /* initialise "this" (i.e. ptThis) to access class members */
    class_internal(ptObj, ptThis, byte_queue_t);

    __protect_queue__ {
        if(this.hwHead + hwCount < this.hwSize){
            this.hwPeek = this.hwHead + hwCount;
        } else{
            this.hwPeek =  hwCount - (this.hwSize - this.hwHead);
        }

        this.hwPeekLength = this.hwPeekLength - hwCount;
    }
    return true;
}

#endif
