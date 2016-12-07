//
// Created by jyh on 16-12-6.
//

#ifndef SERIAL_MAIN_H
#define SERIAL_MAIN_H
#include <unistd.h>
#include <pthread.h>
#include <semaphore.h>
#include <stdlib.h>
#include <stdio.h>

#define BYTE uint8_t
#define WORD uint16_t
//定义串口缓冲
typedef struct {
    BYTE    *addr;
    WORD	size;
    WORD	rp;
    WORD	wp;
}SerBuf;

typedef struct {
    pthread_t thid;
    sem_t sem;//信号量
    int      fd;     /* file id */
    SerBuf rx_buf;
    SerBuf tx_buf;

}Serial;
#endif //SERIAL_MAIN_H
