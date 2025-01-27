#include "AudioTools.h"
#include "BluetoothA2DPSink.h"
#include <Wire.h>

AnalogAudioStream out;
BluetoothA2DPSink a2dp_sink(out);


int CurrentCycle = 0;
int NextCycle = 1;
int PrevCycle = 2;
byte states[] = {0,0,0};
bool triMode = false;
uint8_t track_change_flag = 0;
esp_avrc_playback_stat_t lastPlayState = esp_avrc_playback_stat_t::ESP_AVRC_PLAYBACK_PLAYING;
esp_avrc_playback_stat_t playbackState = esp_avrc_playback_stat_t::ESP_AVRC_PLAYBACK_PLAYING;
bool globalPlayback = true;
bool lastGlobalPlayback = true;


void avrc_metadata_callback(uint8_t data1, const uint8_t *data2) {
  Serial.printf("AVRC metadata rsp: attribute id 0x%x, %s\n", data1, data2);
}
void avrc_rn_playstatus_callback(esp_avrc_playback_stat_t playback) {
  playbackState = playback;
  switch (playback) {
    case esp_avrc_playback_stat_t::ESP_AVRC_PLAYBACK_STOPPED:
      Serial.println("Stopped.");
      break;
    case esp_avrc_playback_stat_t::ESP_AVRC_PLAYBACK_PLAYING:
      Serial.println("Playing.");
      break;
    case esp_avrc_playback_stat_t::ESP_AVRC_PLAYBACK_PAUSED:
      Serial.println("Paused.");
      break;
    case esp_avrc_playback_stat_t::ESP_AVRC_PLAYBACK_FWD_SEEK:
      Serial.println("Forward seek.");
      break;
    case esp_avrc_playback_stat_t::ESP_AVRC_PLAYBACK_REV_SEEK:
      Serial.println("Reverse seek.");
      break;
    case esp_avrc_playback_stat_t::ESP_AVRC_PLAYBACK_ERROR:
      Serial.println("Error.");
      break;
    default:
      Serial.printf("Got unknown playback status %d\n", playback);
  }
}
void avrc_rn_track_change_callback(uint8_t *id) {
  Serial.println("Track Change bits:");
  for (uint8_t i = 0; i < 8; i++)
  {
    Serial.printf("\tByte %d : 0x%x \n",i,id[i]);
  }
  //An example of how to project the pointer value directly as a uint8_t
  track_change_flag = *id;
  Serial.printf("\tFlag value: %d\n",track_change_flag);
}
void setup() {
  Wire.Begin();
  Serial.begin(115200);
  a2dp_sink.set_avrc_metadata_callback(avrc_metadata_callback);
  a2dp_sink.set_avrc_rn_track_change_callback(avrc_rn_track_change_callback)
  a2dp_sink.set_avrc_rn_playstatus_callback(avrc_rn_playstatus_callback)
  a2dp_sink.start("BTCycler1");
  while(!a2dp_sink.is_connected()) 
    delay(1000);
}

void loop() {
}
//state1 nothing, state2 cycle, state3 refersh.
int readState() {
  
  if (playbackState == esp_avrc_playback_stat_t::ESP_AVRC_PLAYBACK_PLAYING){
    states[0] = 1;
  }

  Wire.requestFrom(1,1)
  states[1] = Wire.read()
  if(triMode){
    Wire.requestFrom(1,1)
    states[2] = Wire.read()
  } 
   // if paused or play button is triggered toggle the current device
  if(globalPlayback != lastGlobalPlayback){
    toggle(CurrentCycle);
    Serial.println("Paused = "+String(globalPlayback));
    return 1;
  }
  int playing = 0;
  for(int i = 0; i<3;i++){
    if(states[i] == 0x01){playing++;}
    Serial.println("device " + String(i) + " is " + String(states[i]));
  }
  if(cycleFlag && globalPlayback){return 1;}
  if(playing == 0 && !globalPlayback){
  	return 2;
  }
  else if(playing > 1){
  	return 3;
  }
  else{return 1;}
}