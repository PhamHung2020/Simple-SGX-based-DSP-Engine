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

    // MyEvent(double timestamp, int key, int data, const char* message)
    // {
    //     this->timestamp = timestamp;
    //     this->key = key;
    //     this->data = data;
    //     strncpy(this->message, message, 31);
    // };

    // int getKey()
    // {
    //     return key;
    // };

} MyEvent;


#endif