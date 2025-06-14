#define MAX_TCP_CONNECT         5       /* time in secs to get a connection */
#define MAX_TCP_READ            3       /* time in secs to wait for the DSO
                                           to respond to a read request */
#define BOOL                    int
#define TRUE                    1
#define FALSE                   0

// function prototypes
int MINITCB_V2_Open(char *host);
int MINITCB_V2_Close(int tcp_Handle);
int MINITCB_V2_Transmit(int tcp_Handle, char *buf,int len);
int MINITCB_V2_Receive(int tcp_Handle, char *buf, int len);
void MINITCB_V2_EnableFLASH(int tcp_Handle);
void MINITCB_V2_FinishFLASH(int tcp_Handle);
void MINITCB_V2_EraseFLASH(int tcp_Handle, int sector);
void MINITCB_V2_WriteFLASH(int tcp_Handle, int sector, int addrH, unsigned char *data);
void MINITCB_V2_ReadFLASH(int tcp_Handle, int sector, int addrH, unsigned char *data);
unsigned char MINITCB_V2_StatFLASH(int tcp_Handle);

