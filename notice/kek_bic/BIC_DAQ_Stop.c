#include <unistd.h>
#include <stdio.h>
#include "NoticeBIC_TCB.h"

int main( int argc, char** argv){
    USB3Init();
    int sid = 0;
    TCBopen(sid);
    TCBreset(sid);
    usleep(1000000);
    TCBreset(sid);
    TCBclose(sid);
    USB3Exit();
    return 0;
}