#include <Arduino.h>
#include "ev800d.h"
#include <SPI.h>
#include "logging.h"
#include "rx5808.h"
#include "devDVR.h"

#if defined(DVR_OBSERVER_POLLING)
#define DVR_OBSERVER_REPEAT 30
#endif

static RX5808 _vrxmodule;
static bool dvrState = VRX_DVR_RECORDING_UNKNOWN ;

void ICACHE_RAM_ATTR DvrStateISR()
{
    dvrState = VRX_DVR_RECORDING_ACTIVE;
    digitalWrite(PIN_DVR_STATE_LED, LOW);
}

void 
EV800D::Init()
{
    _vrxmodule.Init();
    #ifdef PIN_DVR
        pinMode(PIN_DVR, OUTPUT);
        digitalWrite(PIN_DVR, VRX_DVR_RECORDING_BUTTON_RELEASE);
        DBGLN("PIN DVR Init"); 
    #endif
    #if defined(PIN_DVR_OBSERVER) && !defined(DVR_OBSERVER_DEVICE)
        pinMode(PIN_DVR_OBSERVER, INPUT);
        DBGLN("PIN_DVR_OBSERVER Init"); 
        #if defined(DVR_OBSERVER_ISR)
            dvrState = VRX_DVR_RECORDING_UNKNOWN;
            attachInterrupt(digitalPinToInterrupt(PIN_DVR_OBSERVER), DvrStateISR, FALLING );
            DBGLN("PIN_DVR_OBSERVER ISR Init");
        #endif
    #endif
    #ifdef PIN_DVR_STATE_LED
    pinMode(PIN_DVR_STATE_LED, OUTPUT);
    digitalWrite(PIN_DVR_STATE_LED, HIGH);
    #endif
    DBGLN("EV800D Init complete");    
}

void 
EV800D::SendIndexCmd(uint8_t index)
{
    _vrxmodule.SendIndexCmd(index);
}

uint8_t 
EV800D::GetChannelIndex()
{
    return 0;
}

void 
EV800D::SetChannelIndex(uint8_t index)
{

}

uint8_t
EV800D::GetRecordingState()
{
    #ifdef PIN_DVR_OBSERVER
        #if defined(DVR_OBSERVER_ISR)
            // ISR 구현하려면 main 합수 device DVR STATE 부터 전부 고쳐야 할듯.
            //SD-CARD CS pin monitor for DVR is activated 
            //MSP_ELRS_BACKPACK_GET_RECORDING_STATE = digitalRead(PIN_DVR_STATE);
            //return MSP_ELRS_BACKPACK_GET_RECORDING_STATE ? VRX_DVR_RECORDING_ACTIVE : VRX_DVR_RECORDING_INACTIVE;
            dvrState = VRX_DVR_RECORDING_INACTIVE;
            digitalWrite(PIN_DVR_STATE_LED, HIGH);
            delayMicroseconds(1000*200);        
        #elif defined(DVR_OBSERVER_DEVICE)
            dvrState = getDvrState();
        #elif defined(DVR_OBSERVER_POLLING)
            uint8_t i = 0 ;
            dvrState = VRX_DVR_RECORDING_INACTIVE;
            digitalWrite(PIN_DVR_STATE_LED, HIGH);
            while ( i < DVR_OBSERVER_REPEAT)
            {
                if( !digitalRead(PIN_DVR_OBSERVER) )
                {
                    dvrState = VRX_DVR_RECORDING_ACTIVE;
                    digitalWrite(PIN_DVR_STATE_LED, LOW);
                }
            }
        #else 
            dvrState = VRX_DVR_RECORDING_UNKNOWN;         
        #endif      
    #endif
    return dvrState;
}

void
EV800D::SetRecordingState(uint8_t recordingState, uint16_t delay)
{
    DBGLN("SetRecordingState = %d delay = %d", recordingState, delay);
    #if defined(DVR_DEVICE)
        #if defined(DVR_OBSERVER_ISR)
        //digitalWrite(PIN_DVR_STATE_LED, ~dvrState);
        uint8_t _state = GetRecordingState() ;
        DBGLN("Check1 %d DVR as Device, State %d ", recordingState, dvrState);
        if(recordingState != _state )
        {
            DBGLN("Check2 %d DVR as Device, State %d ", recordingState, dvrState);
            setDvr(recordingState, delay);    
        }        
        //digitalWrite(PIN_DVR_STATE_LED, dvrState);
        #else
        DBGLN("None %d DVR as Device, State %d ", recordingState, dvrState);
        setDvr(recordingState, delay);   
        #endif
        GetRecordingState() ;
        DBGLN("after %d DVR as Device, State %d ", recordingState, dvrState);
    #elif defined(DVR_NORMAL)
        const uint16_t longPress = 1000;    // 1000ms
        //const uint16_t shortPress = 500;    // 500ms
        //delayMicroseconds(1000*100*delay);

        digitalWrite(PIN_DVR, VRX_DVR_RECORDING_BUTTON_PUSH);
        delayMicroseconds(1000*longPress);
        digitalWrite(PIN_DVR, VRX_DVR_RECORDING_BUTTON_RELEASE); 
        DBGLN("Set DVR as Normal");        
    #endif
}