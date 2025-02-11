//
// Created by hungpm on 25/03/2024.
//

#ifndef EMITTER_H
#define EMITTER_H

class Emitter
{
public:
    virtual ~Emitter() = default;

    virtual void emit(void* data) = 0;
};

#endif //EMITTER_H
