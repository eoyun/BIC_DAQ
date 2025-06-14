#ifndef HV_H
#define HV_H

#define MAX_TCP_CONNECT         5       /* time in secs to get a connection */
#define MAX_TCP_READ            3       /* time in secs to wait for the DSO
                                           to respond to a read request */
#define BOOL                    int
#define TRUE                    1
#define FALSE                   0

#ifdef __cplusplus
extern  "C" {
#endif

extern int HVopen(char *ip);
extern void HVclose(int tcp_Handle);
extern void HVwrite_BASE(int tcp_Handle, float data) ;
extern float HVread_BASE(int tcp_Handle);
extern void HVwrite_CH(int tcp_Handle, int ch, int sipm, float data) ;
extern float HVread_CH(int tcp_Handle, int ch, int sipm);
extern int HVwrite_DAC_ID(int tcp_Handle, int ch, int old_addr, int new_addr);
extern int HVread_DAC_ID(int tcp_Handle, int ch, int dac_addr);

#ifdef __cplusplus
}
#endif

#endif
