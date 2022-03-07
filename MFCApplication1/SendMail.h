// SendMail.h
#ifndef _SEND_MAIL_H_
#define _SEND_MAIL_H_

#include <windows.h>
#include <stdio.h>
#include <WinSock.h>
#include <iostream>
using namespace std;

// 协议中加密部分使用的是base64方法
char ConvertToBase64(char c6);
void EncodeBase64(char *dbuf, char *buf128, int len);
void SendMail(char *email, const char *body);
int  OpenSocket(struct sockaddr *addr);

#endif