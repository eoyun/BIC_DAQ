#include <sys/time.h>
#include <string.h>
#include <unistd.h>
#include <stdio.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netinet/tcp.h>
#include <arpa/inet.h>

#include "programmer_minitcb_v2.h"

// open TCP/IP socket
int MINITCB_V2_Open(char *host)
{
  struct sockaddr_in	serv_addr;
  int tcp_Handle;
  const int disable = 1;
  /*
   * Fill in the structure "serv_addr" with the address of the
   * server that we want to connect with.
   */
        
  //bzero((char *) &serv_addr, sizeof(serv_addr));
  serv_addr.sin_family      = AF_INET;
  serv_addr.sin_addr.s_addr = inet_addr(host);
  serv_addr.sin_port        = htons(5000);
        
  /*
   * Open a TCP socket (an Internet stream socket).
   */
        
  if ( (tcp_Handle = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
    printf("client: can't open stream socket\n");
    return -1;
  }
        
  /* turning off TCP NAGLE algorithm : if not, there is a delay 
     problem (up to 200ms) when packet size is small */ 
  setsockopt(tcp_Handle, IPPROTO_TCP,TCP_NODELAY,(char *) &disable, sizeof(disable)); 

  /*
   * Connect to the server.
   */
        
  if (connect(tcp_Handle, (struct sockaddr *) &serv_addr, sizeof(serv_addr)) < 0) {
    printf("client: can't connect to server\n");
    printf("ip %s , port 5000 \n", host);
    printf("error number is %d \n", connect(tcp_Handle, (struct sockaddr *) &serv_addr,sizeof(serv_addr)));

    return -2;
  } 
  
  return (tcp_Handle);
}

// close TCP/IP socket
int MINITCB_V2_Close(int tcp_Handle) 
{
  close(tcp_Handle);

  return 0;
}

// transmit len byte character stream buf
int MINITCB_V2_Transmit(int tcp_Handle, char *buf,int len)
{
  int result, bytes_more, bytes_xferd;
  char *idxPtr;

  //	BOOL eoi_flag	= TRUE;

  bytes_more = len;
  idxPtr = buf;
  bytes_xferd = 0;
  while (1) {
    /* then write the rest of the block */
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

// receiver len byte character stream buf
int MINITCB_V2_Receive(int tcp_Handle, char *buf, int len)
{
  int result, accum, space_left, bytes_more, buf_count;
  //int i;
  char *idxPtr;

  fd_set rfds;
  struct timeval tval;

  tval.tv_sec = MAX_TCP_READ;
  tval.tv_usec = 0;

  FD_ZERO(&rfds);
  FD_SET(tcp_Handle, &rfds);

  if (buf==NULL)
    return -1;

  //memset(buf, 0, len);

  idxPtr = buf;

  buf_count = 0;
  space_left = len;
  while (1) {
    /* block here until data is received of timeout expires */
    /*
    //      result = select((tcp_Handle+1), &rfds, NULL, NULL, &tval);
    if (result < 0) {
    printf("Read timeout\n");
    return -1;
    }

    printf("Passed Timeout  \n");
    */
    
    /* read the block */
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
      /* in case data is smaller than wanted on */
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

// unprotect flash memory
void MINITCB_V2_EnableFLASH(int tcp_Handle)
{
  char tcpBuf[2];
	
  tcpBuf[0] = 21;

  MINITCB_V2_Transmit(tcp_Handle, tcpBuf, 1);
  MINITCB_V2_Receive(tcp_Handle, tcpBuf, 1);
}

// protect flash memory
void MINITCB_V2_FinishFLASH(int tcp_Handle)
{
  char tcpBuf[2];
	
  tcpBuf[0] = 22;

  MINITCB_V2_Transmit(tcp_Handle, tcpBuf, 1);
  MINITCB_V2_Receive(tcp_Handle, tcpBuf, 1);
}

// erase flash memory
void MINITCB_V2_EraseFLASH(int tcp_Handle, int sector)
{
  char tcpBuf[3];
  unsigned char stat;
	
  tcpBuf[0] = 23;
  tcpBuf[1] = sector & 0xFF;

  MINITCB_V2_Transmit(tcp_Handle, tcpBuf, 2);
  MINITCB_V2_Receive(tcp_Handle, tcpBuf, 1);
   	
  stat = 1;
  while (stat) 
    stat = MINITCB_V2_StatFLASH(tcp_Handle) & 0x01;
}

// write flash memory
void MINITCB_V2_WriteFLASH(int tcp_Handle, int sector, int addrH, unsigned char *data)
{
  char tcpBuf[259];
  int i;
  unsigned char stat;
	
  tcpBuf[0] = 24;
  tcpBuf[1] = sector & 0xFF;
  tcpBuf[2] = addrH & 0xFF;
	
  for (i = 0; i < 256; i++)
    tcpBuf[i + 3] = data[i] & 0xFF;

  MINITCB_V2_Transmit(tcp_Handle, tcpBuf, 259);
  MINITCB_V2_Receive(tcp_Handle, tcpBuf, 1);
   	
  stat = 1;
  while (stat)
    stat = MINITCB_V2_StatFLASH(tcp_Handle) & 0x01;
}

// read flash memory
void MINITCB_V2_ReadFLASH(int tcp_Handle, int sector, int addrH, unsigned char *data)
{
  char tcpBuf[256];
  int i;
	
  tcpBuf[0] = 25;
  tcpBuf[1] = sector & 0xFF;
  tcpBuf[2] = addrH & 0xFF;
	
  MINITCB_V2_Transmit(tcp_Handle, tcpBuf, 3);
  MINITCB_V2_Receive(tcp_Handle, tcpBuf, 256);
	
  for (i = 0; i < 256; i++)
    data[i] = tcpBuf[i] & 0xFF;
}

// read flash memory status
unsigned char MINITCB_V2_StatFLASH(int tcp_Handle)
{
  unsigned char data;
  char tcpBuf[2];
	
  tcpBuf[0] = 26;

  MINITCB_V2_Transmit(tcp_Handle, tcpBuf, 1);
  MINITCB_V2_Receive(tcp_Handle, tcpBuf, 1);
	
  data = tcpBuf[0] & 0xFF;
	
  return data;
}


// Followings are executable routine
int main(void)
{
  char ip[256];
  int tcp_Handle;                // TCP/IP handler
  int com;
  unsigned char data[256];
  int i;
  char filename[256];
  int length;
  FILE *fp;
  char cdat[0x60000];
  int sector;
  int addrH;
  int errcnt;

  printf("enter IP address : ");
  scanf("%s", ip);

  // open MINITCB_V2
  tcp_Handle = MINITCB_V2_Open(ip);

  // read present firmware version
  MINITCB_V2_ReadFLASH(tcp_Handle, 5, 255, data);
  printf("FPGA firmware = %s\n", data);

  com = 99;

  while(com) {
    printf("\n");
    printf("1. upload FPGA firmware\n");
    printf("2. verify FPGA firmware\n");
    printf("0. quit\n");
    printf("enter command : ");
    scanf("%d", &com);

    if (com == 1) {

      printf("enter bit filename : ");
      scanf("%s", filename);

      fp = fopen(filename, "rb");
      if (fp == NULL)
	printf("File not found! Quit without uploading. Bye!\n");
      else {
	fread(cdat, 1, 0x532D8, fp);
	fclose(fp);

        for (i = 0x532D8; i < 0x60000; i++)
          cdat[i] = 0x0;

        length = strlen(filename);

	for (i = 0; i < length; i++)
	  cdat[0x5FF00 + i] = filename[i];

	// enable flash
	MINITCB_V2_EnableFLASH(tcp_Handle);
	
	// write FPGA firmware
	for (sector = 0; sector < 6; sector++) {
	  
	  // erase sector
	  MINITCB_V2_EraseFLASH(tcp_Handle, sector);
	  
	  // write data
	  for (addrH = 0; addrH < 256; addrH++) {
	    for (i = 0; i < 256; i++)
	      data[i] = cdat[sector * 0x10000 + addrH * 0x100 + i] & 0xFF;

            // write flash memory
	    MINITCB_V2_WriteFLASH(tcp_Handle, sector, addrH, data);

	    printf(".");
	    fflush(stdout);
	  }
	}
	
	printf("\n");

	// finish flash
	MINITCB_V2_FinishFLASH(tcp_Handle);
      }
    }
    else if (com == 2) {
      printf("enter bit filename : ");
      scanf("%s", filename);

      fp = fopen(filename, "rb");
      if (fp == NULL)
	printf("File not found! Quit without uploading. Bye!\n");
      else {
        errcnt = 0;
  
	fread(cdat, 1, 0x532D8, fp);
	fclose(fp);

        for (i = 0x532D8; i < 0x60000; i++)
          cdat[i] = 0x0;

        length = strlen(filename);

	for (i = 0; i < length; i++)
	  cdat[0x5FF00 + i] = filename[i];

	// write FPGA firmware
	for (sector = 0; sector < 6; sector++) {
	  
	  // read data
	  for (addrH = 0; addrH < 256; addrH++) {
            MINITCB_V2_ReadFLASH(tcp_Handle, sector, addrH, data);

            // compare data
	    for (i = 0; i < 256; i++) {
              if ((data[i] & 0xFF) != (cdat[sector * 0x10000 + addrH * 0x100 + i] & 0xFF)) {
                errcnt = errcnt + 1;
                printf("%X : %X : %X, write = %X, read = %X, errcnt = %d\n", sector, addrH, i, cdat[sector * 0x10000 + addrH * 0x100 + i] & 0xFF, data[i] & 0xFF, errcnt);
              }
            }
	  }
	}
       
        if (errcnt == 0)
          printf("verification is okay!\n");
      }
    }
  }

  // close MINITCB_V2
  MINITCB_V2_Close(tcp_Handle);

  return 0;
}
  
 







