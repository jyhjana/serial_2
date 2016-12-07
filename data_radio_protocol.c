//
// Created by jyh on 16-12-7.
//

/********************************************************************************
  *@file app_data_radio_protocol.c
  *@brief  和数传电台通信的协议实现
  *
  *
  *@note  备注
  *@warning  注意事项
  *@author Yanhong.Jia
  *@date 2016.09.14
  *@version 版本 1.0.0
  *@since 修改说明
  *******************************************************************************/
#include "data_radio_protocol.h"

radio_protocol* pradioPro = NULL;

uint8_t radio_protocol_init(uint8_t* pbuf)
{
    pradioPro = (radio_protocol*)pbuf;
    memset((void *)pradioPro,0,sizeof(radio_protocol));
    pradioPro->head1 = HEAD1;
    pradioPro->head2 = HEAD2;

    return 1;

}

static uint8_t check_mod(uint8_t* pbuf,uint8_t len)
{
    uint8_t mod = 0;
    uint8_t i = 0;
    for(i =0;i<len;i++)
    {
        mod += pbuf[i];
    }

    return mod&0xFF;

}

uint8_t user_data_pack(uint8_t* pbuf,uint8_t buflen)
{
    uint8_t cmdType =pbuf[DATA_OFFSET] ;

    switch(cmdType)
    {
        case MOVE_AHEAD:

            break ;
        case MOVE_BACK:
            break;
        case MOVE_LEFT:
            break;
        case MOVE_RIGHT:
            break;
        case MOVE_EYE:
            break;
        case MOVE_TENTACLE:
            break;
        case MOVE_MOUTH:
            break;
        default:
            break;

    }
    return 0;
}


uint8_t radio_protocol_pack(uint8_t* pbuf,uint8_t buflen)
{
    uint8_t len = 0;
    if((pbuf[HEAD1_OFFSET] != 0xA5) && (pbuf[HEAD1_OFFSET] != 0x5A))
    {
        return HEAD_ERR;//报文头错误
    }
    len = pbuf[LEN_OFFSET]+5;
    if(buflen<len)//报文没接收完
        return DATA_LOST;

    if(check_mod(pbuf+DSTADDR_OFFSET,pbuf[LEN_OFFSET]) != pbuf[LEN_OFFSET+len])
        return CHECK_ERR;//校验错误


    user_data_unpack(pbuf+DSTADDR_OFFSET,len);

    return 0;

}

uint8_t radio_protocol_ack(uint8_t cmdType,uint8_t addr,uint8_t* pbuf)
{
    radio_protocol_init(pbuf);
    pradioPro->dstAddr = 0;
    pradioPro->len++;
    pradioPro->srcAddr = addr;
    pradioPro->len++;
    pradioPro->data[0] = cmdType;
    pradioPro->len++;
    pradioPro->data[1] = check_mod(&pradioPro->dstAddr,pradioPro->len);

    pradioPro->data[2] = DATAEND;

    return pradioPro->len+5;
}

uint8_t user_data_unpack(uint8_t* pbuf,uint8_t buflen)
{
    uint8_t cmdType =pbuf[DATA_OFFSET] ;

    switch(cmdType)
    {
        case MOVE_AHEAD:

            break ;
        case MOVE_BACK:
            break;
        case MOVE_LEFT:
            break;
        case MOVE_RIGHT:
            break;
        case MOVE_EYE:
            break;
        case MOVE_TENTACLE:
            break;
        case MOVE_MOUTH:
            break;
        default:
            break;

    }

    return 0;
}





uint16_t radio_protocol_unpack(uint8_t* pbuf,uint8_t buflen)
{
    uint8_t len = 0;
    uint8_t datalen = 0;
    if (buflen<1)return LEN_ERR;
    if((pbuf[HEAD1_OFFSET] != 0xA5) && (pbuf[HEAD1_OFFSET] != 0x5A))
    {
        return (1<<8 |HEAD_ERR);//报文头错误
    }
    datalen = pbuf[LEN_OFFSET];
    len = datalen+5;
    if(buflen<len)//报文没接收完
        return DATA_LOST;

    if(check_mod(pbuf+DSTADDR_OFFSET,datalen) != pbuf[LEN_OFFSET+datalen+1])
        return (len<<8 |CHECK_ERR);//校验错误


    user_data_unpack(pbuf+DSTADDR_OFFSET,len);

    return (len<<8 |DATA_OK);

}
uint8_t get_cmdType(uint8_t* pbuf)
{

    return pbuf[DATA_OFFSET];
}

uint8_t get_addr(uint8_t* pbuf)
{

    return pbuf[DSTADDR_OFFSET];
}