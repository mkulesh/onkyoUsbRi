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

#include <string>
#include "OnkyoRi.h"

using namespace StmPlusPlus;

/************************************************************************
 * Class OnkyoRiInputProcessor
 ************************************************************************/

OnkyoRiInputProcessor::OnkyoRiInputProcessor (TimerBase::TimerName _inTimer) :
    command { 0 },
    timer { _inTimer },
    tick { 0 }
{
    timer.startCounter(TIM_COUNTERMODE_UP, System::getMcuFreq()/100000, 0xFFFF, TIM_CLOCKDIVISION_DIV1);
}

int OnkyoRiInputProcessor::processPinIrq (bool pinValue)
{
    uint32_t time = timer.getValue();
    Direction dir = pinValue ? Direction::UP : Direction::DOWN;
    timer.reset();
    if (dir == Direction::UP && time > 250 && time < 350)
    {
        // Header: 3 ms
        return 2;
    }
    else if (dir == Direction::DOWN && time > 150 && time < 250)
    {
        // High bit: 2 ms
        return 1;
    }
    else if (dir == Direction::DOWN && time > 50 && time < 150)
    {
        // Low bit: 1 ms
        return 0;
    }            
    return -1;
}

void OnkyoRiInputProcessor::processMsgStart ()
{
    bits.reset();
    tick = 0;        
    command = 0;
}

bool OnkyoRiInputProcessor::processMsgBit (bool bit)
{
    if (tick > 0 && tick <= RI_BITS_COUNT)
    {
        bits[RI_BITS_COUNT - tick] = bit;
    }
    tick++;
    if (tick > RI_BITS_COUNT)
    {
        tick = 0;
        command = bits.to_ulong();
    }
    return command > 0;
}

/************************************************************************
 * Class OnkyoRiOutputProcessor
 ************************************************************************/

OnkyoRiOutputProcessor::OnkyoRiOutputProcessor (IOPin & _outPin, TimerBase::TimerName _outTimer) :
    command { 0 },
    pin { _outPin },
    timer { _outTimer }
{
    ::memset(usartBuffer, 0, USART_BUFFER_SIZE - 1);
}

bool OnkyoRiOutputProcessor::processUsartIrq()
{
    if (::strlen(usartBuffer) < USART_CMD_LENGHT)
    {
        command = 0;
        return false;
    }
    else
    {
        bool res = hexToDecimal(usartBuffer, command);
        ::memset(usartBuffer, 0, USART_BUFFER_SIZE - 1);
        return res;
    }        
}

void OnkyoRiOutputProcessor::sendBlocking()
{
    timer.startCounter(TIM_COUNTERMODE_UP, System::getMcuFreq()/100000, 0xFFFF, TIM_CLOCKDIVISION_DIV1);
    std::bitset<RI_BITS_COUNT> outBits(command);
    // header
    outTick(300, 100);
    // body
    for (int i = outBits.size() - 1; i >= 0; i--)
    {
        if (outBits[i])
        {
            outTick(100, 200);
        }
        else
        {
            outTick(100, 100);
        }
    }
    // trailer
    outTick(100, 100);   
    timer.stopCounter();
}

void OnkyoRiOutputProcessor::outTick(uint32_t d1, uint32_t d2)
{
    timer.reset();
    pin.setHigh();
    while(timer.getValue() < d1);
    timer.reset();
    pin.setLow();
    while(timer.getValue() < d2);
}

bool OnkyoRiOutputProcessor::hexToDecimal(const char * str, uint32_t & decimal)
{
    size_t length = ::strlen(str);
    if (length == 0 || str[length - 1] == 'x')
    {
        return false;
    }
    decimal = 0;
    uint32_t weight = 1;
    for(int i = length - 1; i >= 0; --i)
    {
        char c = str[i];
        if (c == 'x')
        {
            break;
        }
        
        uint32_t valueOf = 0;
        switch (c)
        {
            case '0': valueOf = 0; break;
            case '1': valueOf = 1; break;
            case '2': valueOf = 2; break;
            case '3': valueOf = 3; break;
            case '4': valueOf = 4; break;
            case '5': valueOf = 5; break;
            case '6': valueOf = 6; break;
            case '7': valueOf = 7; break;
            case '8': valueOf = 8; break;
            case '9': valueOf = 9; break;
            case 'a': case 'A': valueOf = 10; break;
            case 'b': case 'B': valueOf = 11; break;
            case 'c': case 'C': valueOf = 12; break;
            case 'd': case 'D': valueOf = 13; break;
            case 'e': case 'E': valueOf = 14; break;
            case 'f': case 'F': valueOf = 15; break;
            default : return false;
        }
        
        decimal += weight * valueOf;
        weight *= 16;
    }
    return true;
}


