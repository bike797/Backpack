#include <Arduino.h>
#include "common.h"
#include "device.h"
#include "devDVR.h"
#include "logging.h"

#ifdef PIN_DVR

#define DVR_PRESS_BUTTON LOW
#define DVR_RELEASE_BUTTON HIGH
#define DVR_OBSERVER_REFESHRATE 600

static int _controlDelay;
static int _nextDelay=0;
static uint8_t _controlState;
static uint8_t _nextControlState;
static DVRControlState_e _DVRControlState = DVRControlNone;
static bool _DVRActivate = false;
static bool _setDVR = false;
static bool _dvrState = VRX_DVR_RECORDING_UNKNOWN ;

/******************************************************************
 * DVR * 
 * ****************************************************************/

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
        return 10; // delay ms for a getDVRState after button release
    }
    return DURATION_IMMEDIATELY;
}

static void DVR_initialize()
{
    //pinMode(PIN_DVR, OUTPUT);
    //digitalWrite(PIN_DVR, VRX_DVR_RECORDING_BUTTON_RELEASE);        
    pinMode(PIN_DVR, INPUT_PULLUP);
    _DVRControlState = DVRControlNone;
    DBGLN("PIN DVR Init"); 
}

static int DVR_event()
{
    //DBGLN("Call DVR_event %d", _setDVR);
    if (connectionState == running && _setDVR) 
    {
        _setDVR = false;
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
    }    
    return DURATION_NEVER;
}

static int DVR_timeout()
{
    if (_DVRControlState == DVRControlDelay)
    {
        _DVRControlState = DVRControlActive;        
        pinMode(PIN_DVR, OUTPUT);
        return updateDVR();
    } else if (_DVRControlState == DVRControlActive)
    {
        _DVRControlState = DVRControlInactive;
        return updateDVR();
    } else if (_DVRControlState == DVRControlInactive)
    {
        
        #if defined(PIN_DVR_OBSERVER) && defined(DVR_OBSERVER_DEVICE)
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
            pinMode(PIN_DVR, INPUT_PULLUP);
            return DURATION_NEVER;
        }        
    }     
    return DURATION_NEVER;
}

device_t DVRControl_device = {
    .initialize = DVR_initialize,
    .start = NULL,
    .event = DVR_event,
    .timeout = DVR_timeout
};

void setDvr(uint8_t recordingState, uint16_t delay)
{
    _setDVR = true;
    #if defined(PIN_DVR_OBSERVER) && defined(DVR_OBSERVER_DEVICE)
    if (recordingState != getDvrState() )
    {
        _controlDelay = 1000*delay;
        _controlState = recordingState;
        devicesTriggerEvent();        

    }
    #else
        _controlDelay = 1000*delay;
        _controlState = recordingState;
        devicesTriggerEvent();
    #endif
    DBGLN("SetDVR = %d delay = %d", recordingState, _controlDelay);
}

/******************************************************************
 * DVR Observer * 
 * ****************************************************************/

#if defined(PIN_DVR_OBSERVER)

void ICACHE_RAM_ATTR DvrStateISR()
{
    _dvrState = VRX_DVR_RECORDING_ACTIVE;
    _DVRActivate = _dvrState;
    //digitalWrite(PIN_DVR_STATE_LED, LOW);
}

static void DVRObserver_initialize()
{
    pinMode(PIN_DVR_OBSERVER, INPUT);
    DBGLN("PIN_DVR_OBSERVER Init"); 

    #if defined( PIN_DVR_STATE_LED)
    pinMode(PIN_DVR_STATE_LED, OUTPUT);
    digitalWrite(PIN_DVR_STATE_LED, HIGH);
    DBGLN("PIN_DVR_STATE_LED Init"); 
    #endif
        
    _dvrState = VRX_DVR_RECORDING_INACTIVE;
    attachInterrupt(digitalPinToInterrupt(PIN_DVR_OBSERVER), DvrStateISR, CHANGE );
    DBGLN("PIN_DVR_OBSERVER ISR Init");
}

static int DVRObserver_start()
{
    _dvrState = VRX_DVR_RECORDING_INACTIVE;
    return DVR_OBSERVER_REFESHRATE;
}

static int DVRObserver_timeout()
{
    #if defined( PIN_DVR_STATE_LED) 
    digitalWrite(PIN_DVR_STATE_LED, !_dvrState);
    #endif    
    _DVRActivate = _dvrState;
    _dvrState = VRX_DVR_RECORDING_INACTIVE;
    return DVR_OBSERVER_REFESHRATE;
}

device_t DVRObserver_device = {
    .initialize = DVRObserver_initialize,
    .start = DVRObserver_start,
    .event = NULL,
    .timeout = DVRObserver_timeout
};

bool getDvrState()
{
    return _DVRActivate;
}
#endif

#endif