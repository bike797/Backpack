#include <Arduino.h>
#include "common.h"
#include "device.h"
#include "devDVR.h"

#ifdef PIN_DVR

#define DVR_PRESS_BUTTON LOW
#define DVR_RELEASE_BUTTON HIGH
#define DVR_OBSERVER_REFESHRATE 10

static uint32_t _controlDelay = 0;
static uint32_t _nextDelay = 0;
static int _controlState;
static DVRControlState_e _DVRControlState = DVRControlNone;
static DVRObserverState_e _DVRObserverState = DVRObserverUpdate;
static bool _DVRActivate = false;


static int updateDVR()
{
    if (_DVRControlState == DVRControlDelay) return _controlDelay;
    if (_DVRControlState == DVRControlActive) 
    {
        digitalWrite(PIN_DVR, DVR_PRESS_BUTTON);
        return 1000; //over 1000ms delay, hold button for EV800D DVR activation
    }
    if (_DVRControlState == DVRControlInactive) 
    {
        digitalWrite(PIN_DVR, DVR_RELEASE_BUTTON);
        _DVRActivate = false;
        return 1000; // delay ms for a getDVRState after button release
    }
    return DURATION_IMMEDIATELY;
}

static void DVR_initialize()
{
    pinMode(PIN_DVR, OUTPUT);
    digitalWrite(PIN_DVR, DVR_RELEASE_BUTTON);
}

static int DVR_event()
{
    #if defined(PIN_DVR_OBSERVER) && defined(DVR_OBSERVER_DEVICE) 
    if (_controlState != getDvrState())
    {
        if ((_DVRControlState == DVRControlNone ) || (_DVRControlState == DVRControlDelay))
        {
            _DVRControlState = DVRControlDelay;
            _nextDelay = 0;
            return _controlDelay;
        } else {
            _nextDelay = _controlDelay;
            return DURATION_IMMEDIATELY;
        }
    }
    #else
        if ((_DVRControlState == DVRControlNone ) || (_DVRControlState == DVRControlDelay))
        {
            _DVRControlState = DVRControlDelay;
            _nextDelay = 0;
            return _controlDelay;
        } else {
            _nextDelay = _controlDelay;
            return DURATION_IMMEDIATELY;
        }
    #endif
    return DURATION_NEVER;
}

static int DVR_timeout()
{
    if (_DVRControlState == DVRControlDelay)
    {
        _DVRControlState = DVRControlActive;
        return updateDVR();
    } else if (_DVRControlState == DVRControlActive)
    {
        _DVRControlState = DVRControlInactive;
        return updateDVR();
    } else if (_DVRControlState == DVRControlInactive)
    {
        _DVRControlState = DVRControlNone; 
        #if defined(PIN_DVR_OBSERVER) && defined(DVR_OBSERVER_DEVICE)
        _DVRActivate = false;
        #endif
        return DURATION_IMMEDIATELY;
    } else if (_DVRControlState == DVRControlNone)
    {
        if (_nextDelay) 
        {
            _DVRControlState = DVRControlDelay;
            return updateDVR();
        }        
    }
    return DURATION_NEVER;
}

device_t DVR_device = {
    .initialize = DVR_initialize,
    .start = DVR_event,
    .event = DVR_event,
    .timeout = DVR_timeout
};


static int updateDVRObserver()
{
    #if defined(PIN_DVR_OBSERVER) && defined(DVR_OBSERVER_DEVICE)
    if ( !digitalRead(PIN_DVR_OBSERVER) )
    {
        _DVRActivate = true;
    }
    #endif
    return DVR_OBSERVER_REFESHRATE;
}

static void DVRObserver_initialize()
{
    #if defined(PIN_DVR_OBSERVER) && defined(DVR_OBSERVER_DEVICE)
    pinMode(PIN_DVR_OBSERVER, INPUT);
    #endif
}

static int DVRObserver_start()
{
    _DVRObserverState = DVRObserverUpdate;
    return DVR_OBSERVER_REFESHRATE;
}

static int DVRObserver_timeout()
{
    return updateDVRObserver();
}

device_t DVRObserver_device = {
    .initialize = DVRObserver_initialize,
    .start = DVRObserver_start,
    .event = DVRObserver_timeout,
    .timeout = DVRObserver_timeout
};

void setDVR(uint32_t delay, uint8_t state)
{
    #if defined(PIN_DVR_OBSERVER) && defined(DVR_OBSERVER_DEVICE)
    if (state != getDvrState() )
    {
        _controlDelay = delay;
        _controlState = state;
        devicesTriggerEvent();
    }
    #else
        _controlDelay = delay;
        _controlState = state;
        devicesTriggerEvent();
    #endif

}

bool getDvrState()
{
    return _DVRActivate;
}

#endif