#include <Arduino.h>
#include "common.h"
#include "device.h"
#include "devDVR.h"
#include "logging.h"

#ifdef PIN_DVR

#define DVR_PRESS_BUTTON LOW
#define DVR_RELEASE_BUTTON HIGH
#define DVR_OBSERVER_REFESHRATE 10

static int _controlDelay;
static int _nextDelay=0;
static uint8_t _controlState;
static uint8_t _nextControlState;
static DVRControlState_e _DVRControlState = DVRControlNone;
static DVRObserverState_e _DVRObserverState = DVRObserverUpdate;
static bool _DVRActivate = false;


static int updateDVR()
{
    if (_DVRControlState == DVRControlDelay) return _controlDelay;
    if (_DVRControlState == DVRControlActive) 
    {
        digitalWrite(PIN_DVR, DVR_PRESS_BUTTON);
        DBGLN("p");
        return 1000; //over 1000ms delay, hold button for EV800D DVR activation
    }
    if (_DVRControlState == DVRControlInactive) 
    {
        digitalWrite(PIN_DVR, DVR_RELEASE_BUTTON);
        DBGLN("r");
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
            _nextControlState = _controlState;
            return DURATION_IGNORE;
        }
    }
    #else
    {
        if ((_DVRControlState == DVRControlNone ) || (_DVRControlState == DVRControlDelay))
        {
            _DVRControlState = DVRControlDelay;
            _nextDelay = 0;
            return _controlDelay;
        } else {
            _nextDelay = _controlDelay;
            _nextControlState = _controlState;
            return DURATION_IGNORE;
        }
    }
    #endif
    return DURATION_NEVER;
}

static int DVR_timeout()
{
    DBGLN("DVRActivate = %d", _DVRActivate);
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
        #if defined(PIN_DVR_OBSERVER) && defined(DVR_OBSERVER_DEVICE)
        _DVRActivate = false;
        #endif
        if (_nextDelay) 
        {
            _controlDelay = _nextDelay;
            _controlState = _nextControlState;
            _DVRControlState = DVRControlDelay;
            _nextDelay = 0;
            return updateDVR();
        } 
        else 
        {
            _DVRControlState = DVRControlNone; 
            return DURATION_NEVER;
        }        
    }     
    return DURATION_NEVER;
}

device_t DVRControl_device = {
    .initialize = DVR_initialize,
    .start = DVR_timeout,
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
    pinMode(PIN_DVR_OBSERVER, INPUT_PULLUP);
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

void setDvr(uint8_t recordingState, uint16_t delay)
{
    #if defined(PIN_DVR_OBSERVER) && defined(DVR_OBSERVER_DEVICE)
    {
    if (recordingState != getDvrState() )
    {
        _controlDelay = 1000*delay;
        //_controlDelay = 10*1000;
        _controlState = recordingState;
        devicesTriggerEvent();        
        DBGLN("DVR State = %d ", getDvrState());
    }
    }
    #else
    {
        _controlDelay = 1000*delay;
        //_controlDelay = 10*1000;
        _controlState = recordingState;
        devicesTriggerEvent();
    }
    #endif
    DBGLN("SetDVR = %d delay = %d", recordingState, _controlDelay);
}

bool getDvrState()
{
    return _DVRActivate;
}

#endif