//
// Created by jyh on 16-12-7.
//




#include "serial.h"
#include "data_radio_protocol.h"
#include "main.h"

#define SERIAL_BUF_NUM 512
Serial serial;
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


}
void  SerialClose()
{
    UART_Close(serial.fd);
    free(serial.tx_buf.addr);
    free(serial.rx_buf.addr);
    serial.tx_buf.rp = serial.tx_buf.wp = 0;
    serial.rx_buf.rp = serial.rx_buf.wp = 0;


}


void thread_data_proto_proc()
{

    UART_T * uartBuf;
    int i=0;
    uint16_t ret= 0;
    uint8_t revlen = 0;
    uint8_t sedlen = 0;
    uint8_t addr = 0;
    uint8_t tmpchar = 0;
    uint8_t cmdType;
    uint8_t sedbuf[24];
    uartBuf = ComToUart(COM5);
    if(uartBuf->usRxCount>0)
    {
        ret = radio_protocol_unpack(uartBuf->pRxBuf+uartBuf->usRxRead,uartBuf->usRxCount);
        revlen = (ret>>8) & 0xff;
        if((ret & 0xff) == DATA_OK)
        {
            cmdType = get_cmdType(uartBuf->pRxBuf+uartBuf->usRxRead);
            addr = get_addr(uartBuf->pRxBuf+uartBuf->usRxRead);
            sedlen =  radio_protocol_ack(cmdType,addr,sedbuf);
            comSendBuf(COM5,sedbuf,sedlen);
        }
        for(i=0;i<revlen;i++)
        {
            comGetChar(COM5,&tmpchar);
        }

    }

}
int main(int argc, char **argv)
{
    int fd = FALSE;
    int ret;
    char rcv_buf[512];
    int i;

    SerialOpen();
    memset(rcv_buf,0,sizeof(rcv_buf));
    for(i=0;;i++)
    {
        ret = UART_Recv(fd, rcv_buf,512);
        if( ret > 0){
            rcv_buf[ret]='\0';
            printf("%s",rcv_buf);
        } else {
            printf("cannot receive data1\n");
            break;
        }
        if('\n' == rcv_buf[ret-1])
            break;
    }

    while (1){
        ret = UART_Recv(serial.fd, serial.rx_buf.addr+serial.rx_buf.wp,SERIAL_BUF_NUM);
        if( ret > 0){
            rcv_buf[ret]='\0';
            printf("%s",rcv_buf);
        } else {
            printf("cannot receive data1\n");
            break;
        }
    }
    SerialClose();
    return 0;
}