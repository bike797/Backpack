#pragma once

#include <Arduino.h>


#define VRX_DVR_RECORDING_ACTIVE    1
#define VRX_DVR_RECORDING_INACTIVE  0
#define VRX_DVR_RECORDING_UNKNOWN   255
#define VRX_DVR_RECORDING_BUTTON_PUSH LOW
#define VRX_DVR_RECORDING_BUTTON_RELEASE HIGH

void ICACHE_RAM_ATTR DvrStateISR();
class EV800D
{
public:
    void Init();
    void SendIndexCmd(uint8_t index);
    uint8_t GetChannelIndex();
    void SetChannelIndex(uint8_t index);
    uint8_t GetRecordingState();
    void SetRecordingState(uint8_t recordingState, uint16_t delay);
    //bool dvrState = VRX_DVR_RECORDING_UNKNOWN ;


private:        
    //bool dvrState = VRX_DVR_RECORDING_UNKNOWN ;
};