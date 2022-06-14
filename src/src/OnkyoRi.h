/*
 * onkyoUsbRi: Onkyo RI control
 * 
 * Copyright (C) 2021. Mikhail Kulesh
 *
 * This program is free software: you can redistribute it and/or modify it under the terms of the GNU
 * General Public License as published by the Free Software Foundation, either version 3 of the License,
 * or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without
 * even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU General Public License for more details. You should have received a copy of the GNU General
 * Public License along with this program.
 */

#ifndef ONKYO_RI_H_
#define ONKYO_RI_H_

#include <bitset>
#include "BasicIO.h"

namespace StmPlusPlus
{

class OnkyoRiInputProcessor
{
public:
    
    uint32_t command;
   
    OnkyoRiInputProcessor (TimerBase::TimerName _inTimer);
    int processPinIrq (bool pinValue);
    void processMsgStart ();
    bool processMsgBit (bool bit);
    
private:
    
    enum Direction
    {
        UP = 0,
        DOWN = 1
    };
    
    static const size_t RI_BITS_COUNT = 12;
    std::bitset<RI_BITS_COUNT> bits;
    TimerBase timer;
    size_t tick;    
};

class OnkyoRiOutputProcessor
{
public:
    
    uint32_t command;
    
    static const size_t USART_BUFFER_SIZE = 10;
    static const size_t USART_CMD_LENGHT = 6;
    char usartBuffer[USART_BUFFER_SIZE];
    
    OnkyoRiOutputProcessor (IOPin & _outPin, TimerBase::TimerName _outTimer);
    bool processUsartIrq ();
    void sendBlocking ();
    
private:
   
    static const size_t RI_BITS_COUNT = 12;
    IOPin & pin;
    TimerBase timer;

    void outTick (uint32_t d1, uint32_t d2);
    bool hexToDecimal(const char * str, uint32_t & decimal);
};


} // end namespace
#endif
