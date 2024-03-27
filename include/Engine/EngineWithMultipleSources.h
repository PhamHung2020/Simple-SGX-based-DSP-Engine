//
// Created by hungpm on 27/03/2024.
//

#ifndef ENGINEWITHMULTIPLESOURCES_H
#define ENGINEWITHMULTIPLESOURCES_H

#include "SimpleEngine.h"

class EngineWithMultipleSources : public SimpleEngine {
public:
    typedef struct {
        Source source;
        uint16_t enclaveNumber;
    } SourceEnclavePair;

    void addSource(const Source& source, uint16_t enclaveNumber);
    void addTask(uint16_t enclaveNumber, uint16_t callId, uint16_t inputDataSize);

protected:
    std::vector<SourceEnclavePair> sourceEnclavePairs_;

};

#endif //ENGINEWITHMULTIPLESOURCES_H
