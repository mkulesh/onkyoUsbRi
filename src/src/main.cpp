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

#include "OnkyoRi.h"
#include "EventQueue.h"

using namespace StmPlusPlus;

#define USART_DEBUG_MODULE ""

class MyApplication
{
private:
    UsartLogger usart;
    IOPin riLed, mco;
    
    // RI input pin and interrupt
    static const uint16_t RI_INPUT_PIN = GPIO_PIN_1;    
    static const IRQn_Type RI_INPUT_IRQN = EXTI1_IRQn;
    IOPin riInput, riOutput;
    
    // RI command processing
    OnkyoRiInputProcessor inputProcessor;
    OnkyoRiOutputProcessor outputProcessor;

    // Event processing
    enum class Event
    {
        RI_CMD_LOW = 0,
        RI_CMD_HIGH = 1,
        RI_CMD_START = 2,
        USART_INPUT = 3
    };
    StmPlusPlus::EventQueue<Event, 100> eventQueue;

public:
    
    MyApplication () :
        usart(Usart::USART_1, IOPort::B, GPIO_PIN_6, GPIO_PIN_7, GPIO_SPEED_HIGH, 115200),
        riLed(IOPort::A, GPIO_PIN_2, GPIO_MODE_OUTPUT_PP),
        mco(IOPort::A, GPIO_PIN_8, GPIO_MODE_AF_PP),
        riInput(IOPort::B, RI_INPUT_PIN, GPIO_MODE_IT_RISING_FALLING),
        riOutput(IOPort::A, GPIO_PIN_3, GPIO_MODE_OUTPUT_PP, GPIO_NOPULL),
        inputProcessor(TimerBase::TIM_2),
        outputProcessor(riOutput, TimerBase::TIM_3)
    {
        mco.activateClockOutput(RCC_MCO1SOURCE_PLLCLK, RCC_MCODIV_2);
    }
    
    virtual ~MyApplication ()
    {
        // empty
    }
    
    void run ()
    {
        usart.initInstance();
       
        riLed.setHigh();
        USART_DEBUG("--------------------------------------------------------" << UsartLogger::ENDL);
        USART_DEBUG("MCU frequency: " << System::getMcuFreq() << UsartLogger::ENDL);
        riLed.setLow();
        
        // Activate interrupts for RI input
        HAL_NVIC_SetPriority(RI_INPUT_IRQN, 1, 0);
        HAL_NVIC_EnableIRQ(RI_INPUT_IRQN);

        // Activate interrupts for USART
        usart.startInterrupt(InterruptPriority(2, 0));
        usart.receiveIt(outputProcessor.usartBuffer, OnkyoRiOutputProcessor::USART_CMD_LENGHT);
        
        while (true)
        {
            if (!eventQueue.empty())
            {   
                Event event = eventQueue.get();
                switch(event)
                {
                case Event::RI_CMD_START:
                    USART_DEBUG("RI: ");
                    riLed.setHigh();
                    inputProcessor.processMsgStart();
                    break;
                case Event::RI_CMD_LOW:
                case Event::RI_CMD_HIGH:
                    USART_DEBUG("" << int(event))
                    if (inputProcessor.processMsgBit(event == Event::RI_CMD_LOW ? false : true))
                    {
                        USART_DEBUG(" = " << UsartLogger::HEX << inputProcessor.command 
                                    << UsartLogger::DEC << UsartLogger::ENDL);
                        riLed.setLow();
                    }
                    break;
                case Event::USART_INPUT:
                    if (outputProcessor.processUsartIrq())
                    {
                        riLed.setHigh();
                        USART_DEBUG(UsartLogger::ENDL 
                                    << "USART: " << UsartLogger::HEX << outputProcessor.command 
                                    << UsartLogger::DEC << UsartLogger::ENDL);
                        HAL_NVIC_DisableIRQ(RI_INPUT_IRQN);
                        outputProcessor.sendBlocking();
                        HAL_NVIC_EnableIRQ(RI_INPUT_IRQN);
                        riLed.setLow();
                        usart.receiveIt(outputProcessor.usartBuffer, OnkyoRiOutputProcessor::USART_CMD_LENGHT);
                    }
                    break;
                }
            }
            __NOP();
        }
    }
    
    void processRiInputIrq ()
    {
        if (__HAL_GPIO_EXTI_GET_FLAG(RI_INPUT_PIN))
        {
            int val = inputProcessor.processPinIrq(riInput.getBit());
            if (val >= 0)
            {
                eventQueue.put(Event(val));
            }
        }
        HAL_GPIO_EXTI_IRQHandler(RI_INPUT_PIN);
    }
    
    void processUsartIrq ()
    {
        usart.processInterrupt();
        eventQueue.put(Event::USART_INPUT);
    }
};

MyApplication * appPtr = NULL;

int main (void)
{
    // Note: check the Value of the External oscillator mounted in PCB
    // and set this value in the file stm32f3xx_hal_conf.h
    
    HAL_Init();
    
    IOPort defaultPortA(IOPort::PortName::A, GPIO_MODE_INPUT, GPIO_PULLDOWN, GPIO_SPEED_LOW);
    IOPort defaultPortB(IOPort::PortName::B, GPIO_MODE_INPUT, GPIO_PULLDOWN, GPIO_SPEED_LOW);
    IOPort defaultPortC(IOPort::PortName::C, GPIO_MODE_INPUT, GPIO_PULLDOWN, GPIO_SPEED_LOW);
    
    do
    {
        System::setClock(RCC_HSE_PREDIV_DIV2, RCC_PLL_MUL9, FLASH_LATENCY_2, System::RtcType::RTC_INT);
    }
    while (System::getMcuFreq() != 72000000L);
    
    MyApplication app;
    appPtr = &app;
    
    app.run();
}

extern "C" void SysTick_Handler (void)
{
    HAL_IncTick();
}

extern "C" void EXTI1_IRQHandler (void)
{
    appPtr->processRiInputIrq();
}

extern "C" void USART1_IRQHandler (void)
{
    appPtr->processUsartIrq();
}
