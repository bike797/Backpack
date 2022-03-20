#pragma once

#include "device.h"

extern device_t DVRControl_device;
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

void setDvr(uint8_t recordingState, uint16_t delay);
bool getDvrState();