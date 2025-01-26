#include "AudioTools.h"
#include "BluetoothA2DPSink.h"

AnalogAudioStream out;
BluetoothA2DPSink a2dp_sink(out);


int CurrentCycle = 0;
int NextCycle = 1;
int PrevCycle = 2;
bool playback = true;
bool lastPlayState = true;
bool states[] = {0,0,0};
bool triMode = false;

void avrc_metadata_callback(uint8_t data1, const uint8_t *data2) {
  Serial.printf("AVRC metadata rsp: attribute id 0x%x, %s\n", data1, data2);
}
void avrc_rn_playstatus_callback(esp_avrc_playback_stat_t playback) {
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
  uint8_t track_change_flag = *id;
  Serial.printf("\tFlag value: %d\n",track_change_flag);
}
void setup() {
  Serial.begin(115200);
  a2dp_sink.set_avrc_metadata_callback(avrc_metadata_callback);
  a2dp_sink.set_avrc_rn_track_change_callback(avrc_rn_track_change_callback)
  a2dp_sink.start("BTCycler1");
  while(!a2dp_sink.is_connected()) 
    delay(1000);
}

void loop() {
}
int readState() {
  //state1 cycle, state2 refresh, state3 do nothing.
  if(playback != lastPlayState){toggle(CurrentCycle);timeoutCheck(firstInputPin+CurrentCycle, playback);Serial.println("Paused = "+String(playback)); return 3;} // if paused or play button is triggered toggle the current device
  int playing = 0;
  for(int i = 0; i<3;i++){
    int read = digitalRead(firstInputPin+i);
    if(read>=HIGH){
      playing++;
      if(states[i]!=read){
        states[i] = read;
        Serial.println("device " + String(i) + " is " + String(read));
      }  
    }
  }
  if(cycleFlag && playback){return 1;}
  if(playing == 0&&playback){
  	return 1;
  }
  else if(playing > 1){
  	return 2;
  }
  else{return 3;}
}