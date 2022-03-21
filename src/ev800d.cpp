#include <Arduino.h>
#include "ev800d.h"
#include <SPI.h>
#include "logging.h"
#include "rx5808.h"
#include "devDVR.h"

static RX5808 _vrxmodule;

void 
EV800D::Init()
{
    _vrxmodule.Init();
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
    return getDvrState();
}

void
EV800D::SetRecordingState(uint8_t recordingState, uint16_t delay)
{
    uint8_t _dvrState = GetRecordingState();
    DBGLN("SetRecordingState= %d DVRState= %d delay = %d", recordingState, _dvrState, delay);
    #if defined(PIN_DVR_OBSERVER)
    if(recordingState != _dvrState) setDvr(recordingState, delay);
    #else
    setDvr(recordingState, delay);   
    #endif
}