#pragma once

#include "device.h"

extern device_t DVR_device;
extern device_t DVRObserver_device;

typedef enum 
{
    DVRControlDelay,
    DVRControlActive,
    DVRControlInactive,
    DVRControlNone
} DVRControlState_e;

typedef enum 
{
    DVRObserverUpdate,
    DVRObserverReset
} DVRObserverState_e;

void setDvr(uint32_t _delay, uint8_t _state);
bool getDvrState();