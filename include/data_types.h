#ifndef DATA_TYPES_
#define DATA_TYPES_

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

enum ActionType
{
    REDUCE,
    JOIN
};

struct HotOCallParams
{
    // JoinResult joinResult;
    int reduceResult;
    MyEvent eventResult;
};

#endif