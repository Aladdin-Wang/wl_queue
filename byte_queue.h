#ifndef QUEUE_QUEUE_H_
#define QUEUE_QUEUE_H_
#include ".\app_cfg.h"
#if USE_SERVICE_QUEUE == ENABLED
#include <stdint.h>
#include <string.h>
#include <stdbool.h>

/* example:
    #include "./queue/byte_queue.h"
    uint8_t  data1 = 0XAA;
    uint16_t data2 = 0X55AA;
    uint32_t data3 = 0X55AAAA55;
    uint16_t data4[] = {0x1234,0x5678};
    typedef struct data_t{
        uint32_t a;
        uint32_t b;
        uint32_t c;
    }data_t;
    data_t data5 = {
        .a = 0X11223344,
        .b = 0X55667788,
        .c = 0X99AABBCC,
    };

    uint8_t  data[100];
    static uint8_t s_hwQueueBuffer[100];
    static byte_queue_t my_queue;

    QUEUE_INIT(&my_queue,s_hwQueueBuffer,sizeof(s_hwQueueBuffer));

    QUEUE_INIT(&my_queue,s_hwQueueBuffer,sizeof(s_hwQueueBuffer));

    ENQUEUE(&my_queue,data1);//根据变量的类型，自动计算对象的大小
    ENQUEUE(&my_queue,data2);
    ENQUEUE(&my_queue,data3);

    // 一下三种方式都可以正确存储数组
    ENQUEUE(&my_queue,data4,2);//可以不指名数据类型
    ENQUEUE(&my_queue,data4,uint16_t,2);//也可以指名数据类型
    ENQUEUE(&my_queue,data4,uint8_t,sizeof(data4));//或者用其他类型

    //一下两种方式都可以正确存储结构体类型
    ENQUEUE(&my_queue,data5);//根据结构体的类型，自动计算对象的大小
    ENQUEUE(&my_queue,&data5,uint8_t,sizeof(data5));//也可以以数组方式存储

    ENQUEUE(&my_queue,(uint8_t)0X11); //常量默认为int型，需要强制转换数据类型
    ENQUEUE(&my_queue,(uint16_t)0X2233); //常量默认为int型，需要强制转换数据类型
    ENQUEUE(&my_queue,0X44556677);
    ENQUEUE(&my_queue,(char)'a');//单个字符也需要强制转换数据类型
    ENQUEUE(&my_queue,"bc");//字符串默认会存储空字符\0
    ENQUEUE(&my_queue,"def");

    // 读出全部数据
    DEQUEUE(&my_queue,data,GET_QUEUE_COUNT(&my_queue));//DEQUEUE(&my_queue,data,uint8_t,GET_QUEUE_COUNT(&my_queue))

*/

#define __PLOOC_CLASS_USE_STRICT_TEMPLATE__
   
#if     defined(__BYTE_QUEUE_CLASS_IMPLEMENT__)
#   define __PLOOC_CLASS_IMPLEMENT__
#elif   defined(__BYTE_QUEUE_CLASS_INHERIT__)
#   define __PLOOC_CLASS_INHERIT__
#endif   

#include "plooc_class.h"


#ifndef queue_protect
#include "perf_counter.h"
#   define queue_protect() safe_atom_code()
#endif

#define __DEQUEUE_0( __QUEUE, __ADDR)                                \
            dequeue_bytes((__QUEUE), __ADDR,(sizeof(typeof(*(__ADDR)))))

#define __DEQUEUE_1( __QUEUE, __ADDR, __ITEM_COUNT)                        \
            dequeue_bytes((__QUEUE), (__ADDR), __ITEM_COUNT*(sizeof(typeof((__ADDR[0])))))

#define __DEQUEUE_2( __QUEUE, __ADDR, __TYPE,__ITEM_COUNT)                 \
            dequeue_bytes((__QUEUE), (__ADDR), (__ITEM_COUNT * sizeof(__TYPE)))


#define __ENQUEUE_0__( __QUEUE, __VALUE)                                     \
            do{typeof((__VALUE)) SAFE_NAME(value) = __VALUE;                     \
            enqueue_bytes((__QUEUE), &(SAFE_NAME(value)), (sizeof(__VALUE)));}while(0)

#define __ENQUEUE_0( __QUEUE, __VALUE)                                     \
            __ENQUEUE_0__( __QUEUE, __VALUE)

#define __ENQUEUE_1( __QUEUE, __ADDR, __ITEM_COUNT)                         \
            enqueue_bytes((__QUEUE), (__ADDR), __ITEM_COUNT*(sizeof(typeof((__ADDR[0])))))

#define __ENQUEUE_2( __QUEUE, __ADDR, __TYPE, __ITEM_COUNT)                 \
            enqueue_bytes((__QUEUE), (__ADDR), (__ITEM_COUNT * sizeof(__TYPE)))


#define __PEEK_QUEUE_0( __QUEUE, __ADDR)                               \
            peek_bytes_queue((__QUEUE), (__ADDR), (sizeof(typeof(*(__ADDR)))))

#define __PEEK_QUEUE_1( __QUEUE, __ADDR, __ITEM_COUNT)                       \
            peek_bytes_queue((__QUEUE), (__ADDR), __ITEM_COUNT*(sizeof(typeof((__ADDR[0])))))

#define __PEEK_QUEUE_2( __QUEUE, __ADDR, __TYPE, __ITEM_COUNT)               \
            peek_bytes_queue((__QUEUE), (__ADDR), (__ITEM_COUNT * sizeof(__TYPE)))


#define QUEUE_INIT(__QUEUE, __BUFFER, __BUFFER_SIZE )                           \
            queue_init_byte(__QUEUE, __BUFFER, __BUFFER_SIZE )

#define DEQUEUE(__QUEUE, __ADDR,...)                                            \
            __PLOOC_EVAL(__DEQUEUE_,##__VA_ARGS__)                              \
                (__QUEUE,__ADDR,##__VA_ARGS__)                 

#define ENQUEUE(__QUEUE, __ADDR,...)                                            \
            __PLOOC_EVAL(__ENQUEUE_,##__VA_ARGS__)                              \
                (__QUEUE,__ADDR,##__VA_ARGS__) 

#define PEEK_QUEUE(__QUEUE, __ADDR,...)                                         \
            __PLOOC_EVAL(__PEEK_QUEUE_,##__VA_ARGS__)                           \
                (__QUEUE,__ADDR,##__VA_ARGS__) 

#define IS_ENQUEUE_EMPTY(__QUEUE)                                               \
            is_byte_queue_empty(__QUEUE)

#define RESET_PEEK(__QUEUE)                                                     \
            reset_peek_byte(__QUEUE)

#define GET_ALL_PEEKED(__QUEUE)                                                 \
            get_all_peeked_byte(__QUEUE)

#define GET_PEEK_STATUS(__QUEUE)                                                \
            get_peek_status(__QUEUE)

#define RESTORE_PEEK_STATUS(__QUEUE,__COUNT)                                    \
            restore_peek_status(__QUEUE,__COUNT)

#define GET_QUEUE_COUNT(__QUEUE)                                                \
            get_queue_count(__QUEUE)

declare_class(byte_queue_t)
def_class(byte_queue_t,

    private_member(
        uint8_t *pchBuffer;
        uint16_t hwSize;
        uint16_t hwHead;
        uint16_t hwTail;
        uint16_t hwLength;
        uint16_t hwPeek;
        uint16_t hwPeekLength;
    )
    
)
end_def_class(byte_queue_t)

extern byte_queue_t * queue_init_byte(byte_queue_t *ptObj, void *pBuffer, uint16_t hwItemSize);
extern bool enqueue_byte(byte_queue_t *ptQueue, uint8_t chByte);
extern int16_t enqueue_bytes(byte_queue_t *ptObj, void *pDate, uint16_t hwLength);
extern bool dequeue_byte(byte_queue_t *ptQueue, uint8_t *pchByte);
extern int16_t dequeue_bytes(byte_queue_t *ptObj, void *pDate, uint16_t hwLength);
extern bool is_byte_queue_empty(byte_queue_t *ptQueue);
extern bool peek_byte_queue(byte_queue_t *ptQueue, uint8_t *pchByte);
extern bool reset_peek_byte(byte_queue_t *ptQueue);
extern bool get_all_peeked_byte(byte_queue_t *ptQueue);
extern int16_t peek_bytes_queue(byte_queue_t *ptObj, void *pDate, uint16_t hwLength);
extern uint16_t get_peek_status(byte_queue_t *ptQueue);
extern bool restore_peek_status(byte_queue_t *ptQueue ,uint16_t hwCount);
extern int16_t get_queue_count(byte_queue_t *ptObj);
extern int16_t get_queue_available_count(byte_queue_t *ptObj);

#undef __BYTE_QUEUE_CLASS_INHERIT__
#undef __BYTE_QUEUE_CLASS_IMPLEMENT__

#endif
#endif /* QUEUE_QUEUE_H_ */
