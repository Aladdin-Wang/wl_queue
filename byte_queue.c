/****************************************************************************
*  Copyright 2022 KK (https://github.com/WALI-KANG)                                    *
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
#include "byte_queue.h"
#undef this
#define this        (*ptThis)

/****************************************************************************
* Function: queue_init_byte                                               *
* Description: Initializes a byte queue object.                           *
* Parameters:                                                             *
*   - ptObj: Pointer to the byte_queue_t object to be initialized.       *
*   - pBuffer: Pointer to the buffer for storing data.                    *
*   - hwItemSize: Size of each item in the buffer.                        *
*   - bIsCover: Indicates whether the queue should overwrite when full.  *
* Returns: Pointer to the initialized byte_queue_t object or NULL.       *
****************************************************************************/
byte_queue_t *queue_init_byte(byte_queue_t *ptObj, void *pBuffer, uint16_t hwItemSize, bool bIsCover)
{
    assert(NULL != ptObj);
    /* initialise "this" (i.e. ptThis) to access class members */
    byte_queue_t *ptThis = (byte_queue_t *)ptObj;

    if (pBuffer == NULL || hwItemSize == 0) {
        return NULL;
    }

    safe_atom_code() {
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

/****************************************************************************
* Function: reset_queue                                                   *
* Description: Resets the byte queue to its initial state.                *
* Parameters:                                                             *
*   - ptObj: Pointer to the byte_queue_t object to be reset.             *
* Returns: True if the reset is successful, false otherwise.             *
****************************************************************************/
bool reset_queue(byte_queue_t *ptObj)
{
    assert(NULL != ptObj);
    /* initialise "this" (i.e. ptThis) to access class members */
    byte_queue_t *ptThis = (byte_queue_t *)ptObj;
    safe_atom_code() {
        this.hwHead = 0;
        this.hwTail = 0;
        this.hwLength = 0;
        this.hwPeek = this.hwHead;
        this.hwPeekLength = 0;
    }
    return true;
}



/****************************************************************************
* Function: enqueue_bytes                                                 *
* Description: Enqueues multiple bytes into the byte queue.              *
* Parameters:                                                             *
*   - ptObj: Pointer to the byte_queue_t object.                         *
*   - pDate: Pointer to the data to be enqueued.                         *
*   - hwLength: Number of bytes to enqueue.                               *
* Returns: Number of bytes actually enqueued.                             *
****************************************************************************/

uint16_t enqueue_bytes(byte_queue_t *ptObj, void *pDate, uint16_t hwDataLength)
{
    assert(NULL != ptObj);  // Ensure ptObj is not NULL
    assert(NULL != pDate);  // Ensure pDate is not NULL
    /* initialise "this" (i.e. ptThis) to access class members */
    byte_queue_t *ptThis = (byte_queue_t *)ptObj;	
    bool bEarlyReturn = false;  // Initialize early return flag
    safe_atom_code() {  // Start atomic section for thread safety
        if(this.hwHead == this.hwTail && 0 != this.hwLength) {  // Check if queue is full
            if(this.bIsCover == false) {  // If not allowed to overwrite
                bEarlyReturn = true;							
                continue;  // Exit atomic block
            }
        }			
        if(!this.bMutex) {  // Check if mutex is free
            this.bMutex  = true;  // Lock the queue for thread safety
        } else {
            bEarlyReturn = true;  // Another thread is modifying the queue
        }					
    }
    if(bEarlyReturn) {
        return 0;  // Return 0 if queue is full or accessed by another thread
    }		
    uint8_t *pchByte = pDate;  // Cast data pointer to byte pointer
    uint16_t hwTail = this.hwTail;  // Store current tail index
    safe_atom_code() {  // Start atomic section for thread safety
        if(hwDataLength > this.hwSize) {  // If data length exceeds queue size
            hwDataLength = this.hwSize;  // Limit data length to queue size
        }			
        if(hwDataLength > (this.hwSize - this.hwLength)) {  // If not enough space
            if(this.bIsCover == false) {  // If not allowed to overwrite
                hwDataLength = this.hwSize - this.hwLength;  // Adjust data length
            } else {  // If overwriting is allowed
                uint16_t hwOverLength = hwDataLength - (this.hwSize - this.hwLength);  // Calculate overwrite length
                if(hwOverLength < (this.hwSize - this.hwHead)) {
                    this.hwHead += hwOverLength;  // Move head forward
                } else {
                    this.hwHead = hwDataLength - (this.hwSize - this.hwHead);  // Wrap around
                } 
                this.hwLength -= hwOverLength;  // Decrease length
                this.hwPeek = this.hwHead;  // Update peek index
                this.hwPeekLength = this.hwLength;  // Update peek length
            }
        }
        if(hwDataLength < (this.hwSize - this.hwTail)) {
            this.hwTail += hwDataLength;  // Move tail forward
        } else {
            this.hwTail = hwDataLength - (this.hwSize - this.hwTail);  // Wrap around
        }
        this.hwLength += hwDataLength;  // Increase queue length
        this.hwPeekLength += hwDataLength;  // Increase peek length
    } 
    if(hwDataLength <= (this.hwSize - hwTail)) {
        memcpy(&this.pchBuffer[hwTail], pchByte, hwDataLength);  // Copy data to buffer
    } else {
        memcpy(&this.pchBuffer[hwTail], &pchByte[0], this.hwSize - hwTail);  // Copy first part
        memcpy(&this.pchBuffer[0], &pchByte[this.hwSize - hwTail], hwDataLength - (this.hwSize - hwTail));  // Copy second part
    }
    this.bMutex = false;  // Unlock the queue
    return hwDataLength;  // Return number of bytes enqueued
}


/****************************************************************************
* Function: dequeue_bytes                                                 *
* Description: Dequeues multiple bytes from the byte queue.              *
* Parameters:                                                             *
*   - ptObj: Pointer to the byte_queue_t object.                         *
*   - pDate: Pointer to store the dequeued data.                         *
*   - hwLength: Number of bytes to dequeue.                               *
* Returns: Number of bytes actually dequeued.                             *
****************************************************************************/

uint16_t dequeue_bytes(byte_queue_t *ptObj, void *pDate, uint16_t hwDataLength)
{
    assert(NULL != ptObj);  // Ensure ptObj is not NULL
    assert(NULL != pDate);  // Ensure pDate is not NULL

    /* initialise "this" (i.e. ptThis) to access class members */
    byte_queue_t *ptThis = (byte_queue_t *)ptObj;
    bool bEarlyReturn = false;  // Initialize early return flag
    safe_atom_code() {  // Start atomic section for thread safety
        if(this.hwHead == this.hwTail && 0 == this.hwLength) {  // Check if queue is empty
            bEarlyReturn = true;  // Set early return flag
            continue;  // Exit atomic block
        }			
        if(!this.bMutex) {  // Check if mutex is free
            this.bMutex  = true;  // Lock the queue for thread safety
        } else {
            bEarlyReturn = true;  // Another thread is modifying the queue
        }					
    }
    if(bEarlyReturn) {
        return 0;  // Return 0 if queue is empty or accessed by another thread
    }	
    uint8_t *pchByte = pDate;  // Cast data pointer to byte pointer
    uint16_t hwHead = this.hwHead;  // Store current head index
    safe_atom_code() {  // Start atomic section for thread safety
        if(hwDataLength > this.hwLength) {  // If requested length exceeds available data
            hwDataLength = this.hwLength;  // Adjust data length
        }			
        if(hwDataLength < (this.hwSize - this.hwHead)) {
            this.hwHead += hwDataLength;  // Move head forward
        } else {
            this.hwHead = hwDataLength - (this.hwSize - this.hwHead);  // Wrap around
        }					
        this.hwLength -= hwDataLength;  // Decrease queue length
        this.hwPeek = this.hwHead;  // Update peek index
        this.hwPeekLength = this.hwLength;  // Update peek length
    }	
    if(hwDataLength <= (this.hwSize - hwHead)) {
        memcpy(pchByte, &this.pchBuffer[hwHead], hwDataLength);  // Copy data from buffer
    } else {
        memcpy(&pchByte[0], &this.pchBuffer[hwHead], this.hwSize - hwHead);  // Copy first part
        memcpy(&pchByte[this.hwSize - hwHead], &this.pchBuffer[0], hwDataLength - (this.hwSize - hwHead));  // Copy second part
    }			
    this.bMutex = false;  // Unlock the queue
    return hwDataLength;  // Return number of bytes dequeued
}

/****************************************************************************
* Function: is_queue_empty                                                *
* Description: Checks if the byte queue is empty.                         *
* Parameters:                                                             *
*   - ptObj: Pointer to the byte_queue_t object.                         *
* Returns: True if the queue is empty, false otherwise.                  *
****************************************************************************/

bool is_queue_empty(byte_queue_t *ptObj)
{
    assert(NULL != ptObj);
    /* initialise "this" (i.e. ptThis) to access class members */
    byte_queue_t *ptThis = (byte_queue_t *)ptObj;

    if (this.hwHead == this.hwTail &&
        0 == this.hwLength ) {
        return true;
    }

    return false;
}

/****************************************************************************
* Function: get_queue_count                                               *
* Description: Gets the current number of elements in the byte queue.     *
* Parameters:                                                             *
*   - ptObj: Pointer to the byte_queue_t object.                         *
* Returns: Number of elements in the queue.                               *
****************************************************************************/

uint16_t get_queue_count(byte_queue_t *ptObj)
{
    assert(NULL != ptObj);
    /* initialise "this" (i.e. ptThis) to access class members */
    byte_queue_t *ptThis = (byte_queue_t *)ptObj;
    return (this.hwLength);
}
/****************************************************************************
* Function: get_queue_available_count                                     *
* Description: Gets the available space in the byte queue.               *
* Parameters:                                                             *
*   - ptObj: Pointer to the byte_queue_t object.                         *
* Returns: Available space in the queue.                                  *
****************************************************************************/

uint16_t get_queue_available_count(byte_queue_t *ptObj)
{
    assert(NULL != ptObj);
    /* initialise "this" (i.e. ptThis) to access class members */
    byte_queue_t *ptThis = (byte_queue_t *)ptObj;
    return (this.hwSize - this.hwLength);
}

/****************************************************************************
* Function: is_peek_empty                                                 *
* Description: Checks if the peek buffer is empty.                        *
* Parameters:                                                             *
*   - ptObj: Pointer to the byte_queue_t object.                         *
* Returns: True if the peek buffer is empty, false otherwise.            *
****************************************************************************/

bool is_peek_empty(byte_queue_t *ptObj)
{
    assert(NULL != ptObj);
    /* initialise "this" (i.e. ptThis) to access class members */
    byte_queue_t *ptThis = (byte_queue_t *)ptObj;

    if (this.hwPeek == this.hwTail &&
        0 == this.hwPeekLength ) {
        return true;
    }

    return false;
}


/****************************************************************************
* Function: peek_bytes_queue                                              *
* Description: Peeks multiple bytes from the byte queue without dequeuing.*
* Parameters:                                                             *
*   - ptObj: Pointer to the byte_queue_t object.                         *
*   - pDate: Pointer to store the peeked data.                           *
*   - hwLength: Number of bytes to peek.                                  *
* Returns: Number of bytes actually peeked.                               *
****************************************************************************/

uint16_t peek_bytes_queue(byte_queue_t *ptObj, void *pDate, uint16_t hwDataLength)
{
    assert(NULL != ptObj);  // Ensure ptObj is not NULL
    assert(NULL != pDate);  // Ensure pDate is not NULL

    /* initialise "this" (i.e. ptThis) to access class members */
    byte_queue_t *ptThis = (byte_queue_t *)ptObj;
		
    bool bEarlyReturn = false;  // Initialize early return flag
    safe_atom_code() {  // Start atomic section for thread safety
        if(this.hwPeek == this.hwTail && 0 == this.hwPeekLength) {  // Check if peek buffer is empty
            bEarlyReturn = true;  // Set early return flag
            continue;  // Exit atomic block
        }			
        if(!this.bMutex) {  // Check if mutex is free
            this.bMutex  = true;  // Lock the queue for thread safety
        } else {
            bEarlyReturn = true;  // Another thread is modifying the queue
        }					
    }
    if(bEarlyReturn) {
        return 0;  // Return 0 if peek buffer is empty or accessed by another thread
    }
    uint8_t *pchByte = pDate;  // Cast data pointer to byte pointer
    uint16_t hwPeek = this.hwPeek;  // Store current peek index
    safe_atom_code() {  // Start atomic section for thread safety
        if(hwDataLength > this.hwPeekLength) {  // If requested length exceeds available data
            hwDataLength = this.hwPeekLength;  // Adjust data length
        }			
        if(hwDataLength < (this.hwSize - this.hwPeek)) {
            this.hwPeek += hwDataLength;  // Move peek index forward
        } else {
            this.hwPeek = hwDataLength - (this.hwSize - this.hwPeek);  // Wrap around
        }
        this.hwPeekLength -= hwDataLength;  // Decrease peek length
    }
    if(hwDataLength <= (this.hwSize - hwPeek)) {
        memcpy(pchByte, &this.pchBuffer[hwPeek], hwDataLength);  // Copy data from buffer
    } else {
        memcpy(&pchByte[0], &this.pchBuffer[hwPeek], this.hwSize - hwPeek);  // Copy first part
        memcpy(&pchByte[this.hwSize - hwPeek], &this.pchBuffer[0], hwDataLength - (this.hwSize - hwPeek));  // Copy second part
    }
    this.bMutex = false;  // Unlock the queue
    return hwDataLength;  // Return number of bytes peeked
}

/****************************************************************************
* Function: reset_peek                                                    *
* Description: Resets the peek buffer to its initial state.               *
* Parameters:                                                             *
*   - ptObj: Pointer to the byte_queue_t object.                         *
* Returns: True if the reset is successful, false otherwise.             *
****************************************************************************/

bool reset_peek(byte_queue_t *ptObj)
{
    assert(NULL != ptObj);
    /* initialise "this" (i.e. ptThis) to access class members */
    byte_queue_t *ptThis = (byte_queue_t *)ptObj;
    safe_atom_code() {
        this.hwPeek = this.hwHead;
        this.hwPeekLength = this.hwLength;
    }
    return true;
}

/****************************************************************************
* Function: get_all_peeked                                                *
* Description: Moves all peeked elements back to the queue.              *
* Parameters:                                                             *
*   - ptObj: Pointer to the byte_queue_t object.                         *
* Returns: True if successful, false otherwise.                          *
****************************************************************************/

bool get_all_peeked(byte_queue_t *ptObj)
{
    assert(NULL != ptObj);
    /* initialise "this" (i.e. ptThis) to access class members */
    byte_queue_t *ptThis = (byte_queue_t *)ptObj;
    safe_atom_code() {
        this.hwHead = this.hwPeek;
        this.hwLength = this.hwPeekLength;
    }
    return true;
}

/****************************************************************************
* Function: get_peek_status                                               *
* Description: Gets the current status of the peek buffer.               *
* Parameters:                                                             *
*   - ptObj: Pointer to the byte_queue_t object.                         *
* Returns: Current number of elements in the peek buffer.                *
****************************************************************************/

uint16_t get_peek_status(byte_queue_t *ptObj)
{
    assert(NULL != ptObj);
    /* initialise "this" (i.e. ptThis) to access class members */
    byte_queue_t *ptThis = (byte_queue_t *)ptObj;
    uint16_t hwCount;
    safe_atom_code() {
        if (this.hwPeek >= this.hwHead) {
            hwCount = this.hwPeek - this.hwHead;
        } else {
            hwCount = this.hwSize - this.hwHead + this.hwPeek;
        }
    }
    return hwCount;
}

/****************************************************************************
* Function: restore_peek_status                                           *
* Description: Restores the peek buffer status to a previous count.      *
* Parameters:                                                             *
*   - ptObj: Pointer to the byte_queue_t object.                         *
*   - hwCount: Number of elements to restore in the peek buffer.         *
* Returns: True if successful, false otherwise.                          *
****************************************************************************/

bool restore_peek_status(byte_queue_t *ptObj, uint16_t hwCount)
{
    assert(NULL != ptObj);
    /* initialise "this" (i.e. ptThis) to access class members */
    byte_queue_t *ptThis = (byte_queue_t *)ptObj;
    safe_atom_code() {
        if (this.hwHead + hwCount < this.hwSize) {
            this.hwPeek = this.hwHead + hwCount;
        } else {
            this.hwPeek =  hwCount - (this.hwSize - this.hwHead);
        }

        this.hwPeekLength = this.hwPeekLength - hwCount;
    }
    return true;
}
