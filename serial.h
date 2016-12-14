//
// Created by jyh on 16-12-5.
//
#ifndef _SERIAL__H
#define _SERIAL__H

int set_opt(int fd,int nSpeed, int nBits, char nEvent, int nStop);
int open_port(int fd,int comport);
#endif //SERIAL_SER