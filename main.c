//
// Created by jyh on 16-12-7.
//


#include <stdio.h>
#include <stdlib.h>
#include <signal.h>

#include "serial.h"
#include "data_radio_protocol.h"
#include "main.h"

#define SERIAL_BUF_NUM 512
Serial serial;
pthread_mutex_t mutex;
void SerialOpen()
{
    int ret;
    serial.fd = UART_Open(serial.fd,"/dev/ttyUSB0");
    if(FALSE == serial.fd){
        printf("open error\n");
        exit(1);
    }
    ret = UART_Init(serial.fd,9600,0,8,1,'N');
    if (FALSE == ret){
        printf("Set Port Error\n");
        exit(1);
    }
    serial.tx_buf.addr = (BYTE*)malloc(SERIAL_BUF_NUM*sizeof(BYTE));
    serial.tx_buf.rp = serial.tx_buf.wp = 0;
    serial.tx_buf.size = SERIAL_BUF_NUM;

    serial.rx_buf.addr = (BYTE*)malloc(SERIAL_BUF_NUM*sizeof(BYTE));
    serial.rx_buf.rp = serial.rx_buf.wp = 0;
    serial.rx_buf.size = SERIAL_BUF_NUM;

    sem_init (&serial.sem, 0, 1);

}
void  SerialClose()
{
    UART_Close(serial.fd);
    free(serial.tx_buf.addr);
    free(serial.rx_buf.addr);
    serial.tx_buf.rp = serial.tx_buf.wp = 0;
    serial.rx_buf.rp = serial.rx_buf.wp = 0;
    sem_destroy (&serial.sem);

}


void* pthread_data_proto_proc(void * msg)
{

    SerBuf * uartBuf;
    int i=0;
    uint16_t ret= 0;
    uint16_t recvlen = 0;
    uint16_t sendlen = 0;
    uint16_t datalen = 0;
    uint16_t status = DATA_OK;
    uint8_t addr = 0;
    uint8_t cmdType;
    while (1)
    {
        sem_wait (&serial.sem);
        uartBuf = &serial.rx_buf;
        while (status != DATA_LOST)
        {
            recvlen = (uartBuf->wp - uartBuf->rp + SERIAL_BUF_NUM)%SERIAL_BUF_NUM;
            ret = radio_protocol_unpack(uartBuf->addr+uartBuf->rp,recvlen);
            datalen = (ret>>8) & 0xff;
            uartBuf->rp += datalen;
            status = ret & 0x00ff;
            switch (status)
            {
                case DATA_OK:
                    cmdType = get_cmdType(uartBuf->addr+uartBuf->rp);
                    addr = get_addr(uartBuf->addr+uartBuf->rp);
                    sendlen =  radio_protocol_ack(cmdType,addr,serial.tx_buf.addr+serial.tx_buf.wp);
                    serial.tx_buf.wp += sendlen;
                    ret = UART_Send(serial.fd,serial.tx_buf.addr+serial.tx_buf.rp,sendlen);
                    if(ret==sendlen);
                    serial.tx_buf.rp = serial.tx_buf.wp = 0;
                    break;
                case DATA_LOST:

                    break;
                default:

                    break;
            }
            for(i=0;i<recvlen-datalen;i++)
            {
                *(uartBuf->addr+i)=*(uartBuf->addr+ uartBuf->rp+i);

            }
            serial.rx_buf.rp = 0;
            pthread_mutex_lock(&mutex);
            serial.rx_buf.wp -= datalen;
            pthread_mutex_unlock(&mutex);

        }

    }


}
// 程序退出时的函数操作
void sighander(int n,struct siginfo *siginfo,void *myact)
{
//    printf("signal number:%d\n",n);/** 打印出信号值 **/
//    printf("siginfo signo:%d\n",siginfo->si_signo); /** siginfo结构里保存的信号值 **/
//    printf("siginfo errno:%d\n",siginfo->si_errno); /** 打印出错误代码 **/
//    printf("siginfo code:%d\n",siginfo->si_code);   /**　打印出出错原因 **/
    exit(0);
}
int main(int argc, char **argv)
{
    int fd = FALSE;
    int ret;

    int i;

    /** install signal use sigaction **/
    struct sigaction act;
    sigemptyset(&act.sa_mask);   /** 清空阻塞信号 **/
    act.sa_flags=SA_SIGINFO;     /** 设置SA_SIGINFO 表示传递附加信息到触发函数 **/
    act.sa_sigaction=sighander;
    if(sigaction(SIGINT,&act,NULL) < 0)
    {
        printf("install signal error\n");
    }

    pthread_mutex_init(&mutex, NULL);
    SerialOpen();
    ret = pthread_create (&serial.thid, NULL, (void*)pthread_data_proto_proc, NULL);
    if (ret != 0)
    {
        perror ("Create thread failed!");
        pthread_mutex_destroy(&mutex);
        exit(1);
    }

    while (1)
    {
        ret = UART_Recv(serial.fd, serial.rx_buf.addr+serial.rx_buf.wp,SERIAL_BUF_NUM);
        if( ret > 0){
            pthread_mutex_lock(&mutex);
            serial.rx_buf.wp += ret;
            pthread_mutex_unlock(&mutex);
            sem_post (&serial.sem);
        } else {
            printf("cannot receive data1\n");
        }
    }
    SerialClose();
    (void)pthread_mutex_unlock(&mutex);
    pthread_mutex_destroy(&mutex);
    pthread_join(serial.fd, NULL);

    return 0;
}