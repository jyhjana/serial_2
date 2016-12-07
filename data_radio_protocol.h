//
// Created by jyh on 16-12-5.
//

#ifndef SERIAL_DATA_RADIO_PROTOCOL_H
#define SERIAL_DATA_RADIO_PROTOCOL_H
/********************************************************************************
  *@file app_datachange.h
  *@brief 与主控系统板通信功能函数声明
  *
  *
  *@note  备注
  *@warning  注意事项
  *@author Yanhong.Jia
  *@date 2016.06.17
  *@version 版本 1.0.0
  *@since 修改说明
  *******************************************************************************/

#include "string.h"
#include "stdint.h"
#include "stdio.h"
//动作数据类型编码
#define MOVE_AHEAD 	0x01
#define MOVE_BACK  	0x02
#define MOVE_LEFT 		0x03
#define MOVE_RIGHT 	0x04
#define MOVE_EYE   		0x05
#define MOVE_TENTACLE  0x06
#define MOVE_MOUTH  	0x07


//协议结构定义
#define DATALEN 3

#define HEAD1_OFFSET 0
#define HEAD2_OFFSET 1
#define LEN_OFFSET 2
#define DSTADDR_OFFSET LEN_OFFSET+1
#define SRCADDR_OFFSET DSTADDR_OFFSET+1
#define DATA_OFFSET    SRCADDR_OFFSET+1

typedef struct{
    uint8_t head1;
    uint8_t head2;
    uint8_t len;
    uint8_t dstAddr;
    uint8_t srcAddr;
    uint8_t data[DATALEN+2];
}radio_protocol;

//协议定义
#define HEAD1 0xA5
#define HEAD2 0x5A
#define DATAEND 0xAA

//协议错误定义
typedef enum{
    DATA_OK,
    HEAD_ERR,
    LEN_ERR,
    DATA_LOST,
    CHECK_ERR
}ERR_TYPE;
uint8_t user_data_unpack(uint8_t* pbuf,uint8_t buflen);


uint16_t radio_protocol_unpack(uint8_t* pbuf,uint8_t buflen);
uint8_t radio_protocol_ack(uint8_t cmdType,uint8_t addr,uint8_t* pbuf);

uint8_t get_cmdType(uint8_t* pbuf);
uint8_t get_addr(uint8_t* pbuf);





#endif //SERIAL_DATA_RADIO_PROTOCOL_H
