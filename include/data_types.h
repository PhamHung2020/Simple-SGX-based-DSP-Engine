#ifndef __DATA_TYPES_
#define __DATA_TYPES_

#include <string.h>

typedef struct
{
    double timestamp;
    int sourceId;
    int key;
    int data;
    char message[32];
} MyEvent;

typedef struct
{
    MyEvent event1;
    MyEvent event2;
} JoinResult;

#endif