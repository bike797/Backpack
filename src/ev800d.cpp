#include "ev800d.h"
#include <SPI.h>
#include "logging.h"
#include "rx5808.h"
#include "devDVR.h"

#if defined(DVR_OBSERVER_POLLING)
#define DVR_OBSERVER_REPEAT 30
#endif

static RX5808 _vrxmodule;

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
        pinMode(PIN_DVR_OBSERVER, INPUT_PULLUP);
        DBGLN("PIN_DVR_OBSERVER Init"); 
        #if defined(DVR_OBSERVER_ISR)
            dvrState = VRX_DVR_RECORDING_UNKNOWN;
            //attachInterrupt(digitalPinToInterrupt(PIN_DVR_OBSERVER), EV800D::DvrStateISR, CHANGE );
            DBGLN("PIN_DVR_OBSERVER ISR Init");
        #endif
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
            delayMicroseconds(1000*100);        
        #elif defined(DVR_OBSERVER_DEVICE)
            dvrState = getDvrState();
        #elif defined(DVR_OBSERVER_POLLING)
            uint8_t i = 0 ;
            dvrState = VRX_DVR_RECORDING_INACTIVE;
            while ( i < DVR_OBSERVER_REPEAT)
            {
                if( !digitalRead(PIN_DVR_OBSERVER) )
                {
                    dvrState = VRX_DVR_RECORDING_ACTIVE;
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
        setDvr(recordingState, delay);   
        DBGLN("Set DVR as Device");
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