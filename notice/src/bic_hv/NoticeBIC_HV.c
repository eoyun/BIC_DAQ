#include <sys/time.h>
#include <string.h>
#include <unistd.h>
#include <stdio.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netinet/tcp.h>
#include <arpa/inet.h>
#include "NoticeBIC_HV.h"

// internal functions ********************************************************************************
static int HVtransmit(int tcp_Handle, char *buf, int len);
static int HVreceive(int tcp_Handle, char *buf, int len);

// transmit characters to HV
static int HVtransmit(int tcp_Handle, char *buf, int len)
{
  int result;
  int bytes_more;
  int  bytes_xferd;
  char *idxPtr;

  bytes_more = len;
  idxPtr = buf;
  bytes_xferd = 0;
  while (1) {
    idxPtr = buf + bytes_xferd;
    result=write (tcp_Handle, (char *) idxPtr, bytes_more);

    if (result<0) {
      printf("Could not write the rest of the block successfully, returned: %d\n",bytes_more);
      return -1;
    }
    
    bytes_xferd += result;
    bytes_more -= result;
    
    if (bytes_more <= 0)
      break;
  }

  return 0;
}

// receive characters from HV
static int HVreceive(int tcp_Handle, char *buf, int len)
{
  int result;
  int accum;
  int space_left;
  int bytes_more;
  int buf_count;
  char *idxPtr;

  fd_set rfds;
  struct timeval tval;

  tval.tv_sec = MAX_TCP_READ;
  tval.tv_usec = 0;

  FD_ZERO(&rfds);
  FD_SET(tcp_Handle, &rfds);

  if (buf==NULL)
    return -1;

  idxPtr = buf;

  buf_count = 0;
  space_left = len;
  while (1) {
    accum = 0;
    while (1) {
      idxPtr = buf + (buf_count + accum);
      bytes_more = space_left;
      
      if ((result = read(tcp_Handle, (char *) idxPtr, (bytes_more>2048)?2048:bytes_more)) < 0) {
        printf("Unable to receive data from the server.\n");
        return -1;
      }
      
      accum += result;
      if ((accum + buf_count) >= len)
	break;

      if(result<bytes_more) {
        printf("wanted %d got %d \n",bytes_more,result);
        return accum+buf_count;
      }
    }
    
    buf_count += accum;
    space_left -= accum;

    if (space_left <= 0)
      break;
  }

  return buf_count;
}

// ***************************************************************************************************

// open HV
int HVopen(char *ip)
{
  struct sockaddr_in serv_addr;
  int tcp_Handle;
  const int disable = 1;
        
  serv_addr.sin_family      = AF_INET;
  serv_addr.sin_addr.s_addr = inet_addr(ip);
  serv_addr.sin_port        = htons(5000);
        
  if ( (tcp_Handle = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
    printf("Can't open HV\n");
    return -1;
  }

  setsockopt(tcp_Handle, IPPROTO_TCP,TCP_NODELAY,(char *) &disable, sizeof(disable)); 

  if (connect(tcp_Handle, (struct sockaddr *) &serv_addr, sizeof(serv_addr)) < 0) {
    printf("client: can't connect to server\n");
    printf("ip %s , port 5000 \n", ip);
    printf("error number is %d \n", connect(tcp_Handle, (struct sockaddr *) &serv_addr,sizeof(serv_addr)));

    return -2;
  } 
  
  return tcp_Handle;
}

// close HV
void HVclose(int tcp_Handle)
{
  close(tcp_Handle);
}

// write Base HV
void HVwrite_BASE(int tcp_Handle, float data) 
{
  char tcpBuf[3];
  float fval;
  int ival;
  
  fval = data / 53.7 * 4096.0;
  ival = (int)(fval);
  if (ival > 4095)
    ival = 4095;
  else if (ival < 0)
    ival = 0;
    
  tcpBuf[0] = 1;
  tcpBuf[1] = ival & 0xFF;
  tcpBuf[2] = (ival >> 8) & 0xF;
  HVtransmit(tcp_Handle, tcpBuf, 3);
  
  HVreceive(tcp_Handle, tcpBuf, 1);
}

// read Base HV
float HVread_BASE(int tcp_Handle)
{
  char tcpBuf[2];
  int ival;
  float fval;

  tcpBuf[0] = 2;
  HVtransmit(tcp_Handle, tcpBuf, 1);
  
  HVreceive(tcp_Handle, tcpBuf, 2);
  ival = tcpBuf[1] & 0xFF;
  ival = ival << 8;
  ival = ival + (tcpBuf[0] & 0xFF);
  
  fval = ival;
  fval = fval / 4096.0 * 53.7;
  
  return fval;
}

// write Channel HV
void HVwrite_CH(int tcp_Handle, int ch, int sipm, float data) 
{
  char tcpBuf[5];
  float fval;
  int ival;
  
  fval = data / 8.233 * 4096.0;
  ival = (int)(fval);
  if (ival > 4095)
    ival = 4095;
  else if (ival < 0)
    ival = 0;
    
  tcpBuf[0] = 3;
  tcpBuf[1] = ch & 0xFF;
  tcpBuf[2] = sipm & 0xFF;
  tcpBuf[3] = ival & 0xFF;
  tcpBuf[4] = (ival >> 8) & 0xF;
  HVtransmit(tcp_Handle, tcpBuf, 5);
  
  HVreceive(tcp_Handle, tcpBuf, 1);
}

// read Channel HV
float HVread_CH(int tcp_Handle, int ch, int sipm)
{
  char tcpBuf[3];
  int ival;
  float fval;

  tcpBuf[0] = 4;
  tcpBuf[1] = ch & 0xFF;
  tcpBuf[2] = sipm & 0xFF;
  HVtransmit(tcp_Handle, tcpBuf, 3);
  
  HVreceive(tcp_Handle, tcpBuf, 2);
  ival = tcpBuf[1] & 0xFF;
  ival = ival << 8;
  ival = ival + (tcpBuf[0] & 0xFF);
  
  fval = ival;
  fval = fval / 4096.0 * 8.233;
  
  return fval;
}

// write DAC ID
int HVwrite_DAC_ID(int tcp_Handle, int ch, int old_addr, int new_addr)
{
  int response;
  char tcpBuf[4];
  
  tcpBuf[0] = 5;
  tcpBuf[1] = ch & 0xFF;
  tcpBuf[2] = old_addr & 0xFF;
  tcpBuf[3] = new_addr & 0xFF;
  HVtransmit(tcp_Handle, tcpBuf, 4);
  
  HVreceive(tcp_Handle, tcpBuf, 1);
  response = tcpBuf[0] & 0xFF;
  
  return response;  
}

// read DAC ID
int HVread_DAC_ID(int tcp_Handle, int ch, int dac_addr)
{
  int data;
  char tcpBuf[3];
  
  tcpBuf[0] = 6;
  tcpBuf[1] = ch & 0xFF;
  tcpBuf[2] = dac_addr & 0xFF;
  HVtransmit(tcp_Handle, tcpBuf, 3);
  
  HVreceive(tcp_Handle, tcpBuf, 1);
  data = tcpBuf[0] & 0xFF;
  
  return data;  
}





