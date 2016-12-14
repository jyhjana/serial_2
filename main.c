//
// Created by jyh on 16-12-7.
//



#include <signal.h>

#include "data_radio_protocol.h"
#include "main.h"
#include "serial.h"

#define SERIAL_BUF_NUM 512
Serial serial;
pthread_mutex_t mutex;


static void _DumpLine(long addr, unsigned char* buf, int len)
{
    int i, pos;
    char line[80+1];

    // Address field
    pos = sprintf(line, "%08X ", addr);

    // Hex content
    for (i = 0; i < 16; ++i) {
        if (i % (16/2) == 0) {
            line[pos++] = ' '; // Insert a space
        }

        if (i < len) {
            pos += sprintf(&line[pos], "%02x ", buf[i]);
        } else {
            pos += sprintf(&line[pos], "   ");
        }
    }
    pos += sprintf(&line[pos], " |");

    // Printable content
    for (i = 0; i < len; ++i) {
        line[pos++] = isprint(buf[i]) ? buf[i] : '.';
    }

    sprintf(&line[pos], "|\n");
    fprintf(stderr, "%s", line);

}

void DumpHex(unsigned char* buf, int len)
{
    int i;
    for (i = 0; i < (len/16); ++i) {
        _DumpLine(16*i, &buf[16*i], 16);
    }
    // Dump remaining which len is not 16
    if (len % 16 != 0) {
        _DumpLine(16*i, &buf[16*i], len % 16);
    }
}

int SerialOpen()
{
    int ret;

    /*打开串口*/
    if((serial.fd = open_port(serial.fd,1)) < 0)
    {
        perror("open_port error!\n");
        return -1;
    }
    /*设置串口*/
    if((ret= set_opt(serial.fd,115200,8,'N',1)) < 0)
    {
        perror("set_opt error!\n");
        return (-1);
    }

    serial.tx_buf.addr = (BYTE*)malloc(SERIAL_BUF_NUM*sizeof(BYTE));
    serial.tx_buf.rp = serial.tx_buf.wp = 0;
    serial.tx_buf.size = SERIAL_BUF_NUM;

    serial.rx_buf.addr = (BYTE*)malloc(SERIAL_BUF_NUM*sizeof(BYTE));
    serial.rx_buf.rp = serial.rx_buf.wp = 0;
    serial.rx_buf.size = SERIAL_BUF_NUM;

    sem_init (&serial.sem, 0, 1);
    return 0;

}
void  SerialClose()
{
    close(serial.fd);
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
        status = DATA_OK;
        sem_wait (&serial.sem);
        uartBuf = &serial.rx_buf;
        if (uartBuf->wp != uartBuf->rp)
        {
            recvlen = (uartBuf->wp - uartBuf->rp + SERIAL_BUF_NUM)%SERIAL_BUF_NUM;
            ret = radio_protocol_unpack(uartBuf->addr+uartBuf->rp,recvlen);
            datalen = (ret>>8) & 0xff;
            uartBuf->rp += datalen;
            status = ret & 0x00ff;
            if (status == DATA_OK)
            {
                cmdType = get_cmdType(uartBuf->addr+uartBuf->rp);
                addr = get_addr(uartBuf->addr+uartBuf->rp);
                sendlen =  radio_protocol_ack(cmdType,addr,serial.tx_buf.addr+serial.tx_buf.wp);
                //serial.tx_buf.wp += sendlen;
                ret = write(serial.fd,serial.tx_buf.addr+serial.tx_buf.rp,sendlen);
                //if(ret==sendlen);
                //serial.tx_buf.rp = serial.tx_buf.wp = 0;

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
void sighandle(int sig)
{
    SerialClose();
    (void)pthread_mutex_unlock(&mutex);
    pthread_mutex_destroy(&mutex);
    pthread_join(serial.fd, NULL);
    exit(0);
}
int main(int argc, char **argv)
{
    int ret;
    int i;
    int nread;
    char buff[128];
    fd_set rd;

    signal(SIGINT, sighandle);  //设置信号处理函数
    pthread_mutex_init(&mutex, NULL);

    SerialOpen();

    ret = pthread_create (&serial.thid, NULL, (void*)pthread_data_proto_proc, NULL);
    if (ret != 0)
    {
        perror ("Create thread failed!");
        pthread_mutex_destroy(&mutex);
        exit(1);
    }
    /*利用select函数来实现多个串口的读写*/
    while(1)
    {
        FD_ZERO(&rd);
        FD_SET(serial.fd,&rd);
        while(FD_ISSET(serial.fd,&rd))
        {
            if(select(serial.fd+1,&rd,NULL,NULL,NULL) < 0)
                perror("select error!\n");
            else
            {
                while((nread = read(serial.fd,buff,128))>0)
                {

                    DumpHex(buff,nread);
                    pthread_mutex_lock(&mutex);
                    for (i = 0; i < nread; ++i)
                    {
                        serial.rx_buf.addr[serial.rx_buf.wp] = buff[i];
                        serial.rx_buf.wp = (serial.rx_buf.wp+1)%SERIAL_BUF_NUM;
                    }
                    pthread_mutex_unlock(&mutex);
                    sem_post (&serial.sem);
                }
            }
        }
    }
    close(serial.fd);
    return 0;
}