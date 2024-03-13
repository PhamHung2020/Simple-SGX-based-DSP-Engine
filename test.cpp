#include <iostream>

#include "App/CsvSource.h"

typedef struct
{
    double timestamp;
    int key;
    int data;
    char message[32];

    int getKey()
    {
        return key;
    }
} CustomEvent1;

typedef struct
{
    double timestamp;
    int key;
    int data;
    char message[32];
} CustomEvent2;

struct JoinResult
{
    int key;
    int data1;
    int data2;

    JoinResult() 
    {
        key = data1 = data2 = 0;
    }
};

int nestedLoopJoin(CustomEvent1* event1s, CustomEvent2* event2s, int n1, int n2, JoinResult* &results)
{
    results = new JoinResult[n1 * n2];
    int nItems = 0;
    for (int i = 0; i < n1; ++i)
    {
        for (int j = 0; j < n2; ++j)
        {
            if (event1s[i].key == event2s[j].key)
            {
                results[nItems].key = event1s[i].key;
                results[nItems].data1 = event1s[i].data;
                results[nItems].data2 = event2s[j].data;
                nItems++;
            }
        }
    }

    return nItems;
}

void writeEvent(MyEvent event)
{
    std::cout << event.timestamp << " " << event.sourceId << " " << event.key << " " << event.data << " " <<  event.message << std::endl;
}

int main()
{
    CsvSource source(1, "test_data.csv", -1);
    int status = source.start(writeEvent);
    std::cout << "Status: " << status << std::endl;













    // int n1 = 5;
    // int n2 = 10;

    // CustomEvent1* event1s = new CustomEvent1[n1];
    // for (int i = 0; i < n1; ++i)
    // {
    //     event1s[i] = { i * 0.1, i, i * 2 + 5, "Event 1" };
    // }

    // CustomEvent2* event2s = new CustomEvent2[n2];
    // for (int i = 0; i < n2; ++i)
    // {
    //     event2s[i] = { i * 0.2, i * 2, i * 2 + 5, "Event 2" };
    // }

    // JoinResult* results;
    // int nItems = nestedLoopJoin(event1s, event2s, n1, n2, results);

    // printf("%d items matched\n", nItems);
    // for (int i = 0; i < nItems; ++i)
    // {
    //     JoinResult result = results[i];
    //     printf("%d %d %d\n", result.key, result.data1, result.data2);
    // }

    // delete[] event1s;
    // delete[] event2s;
    // delete[] results;

    return 0;
}