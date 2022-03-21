#pragma once

#include "device.h"

#define VRX_DVR_RECORDING_ACTIVE    1
#define VRX_DVR_RECORDING_INACTIVE  0
#define VRX_DVR_RECORDING_UNKNOWN   255

extern device_t DVRControl_device;
extern device_t DVRObserver_device;

typedef enum 
{
    DVRControlDelay,
    DVRControlActive,
    DVRControlInactive,
    DVRControlNone
} DVRControlState_e;

void setDvr(uint8_t recordingState, uint16_t delay);
bool getDvrState();