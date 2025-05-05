//
// Created by katharsis on 5/2/25.
//

#ifndef MEMORYSLAVE_H
#define MEMORYSLAVE_H

class MemorySlave{
public:
    virtual int read_request(int address)=0;
    virtual void write_request(int addr, int value)=0;
};
#endif //MEMORYSLAVE_H
