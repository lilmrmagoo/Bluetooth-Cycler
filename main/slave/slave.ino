#include "AudioTools.h"
#include "BluetoothA2DPSink.h"
#include <Wire.h>

enum connection_state {WAITING = 0x01 ,PAIRING,CONNECTED};

I2SStream i2s;
BluetoothA2DPSink a2dp_sink(i2s);
uint8_t track_change_flag = 0;
bool cycleFlag = false;
connection_state connectionState = PAIRING;
byte selectedReg = 0x00;
bool toggleFlag = false;
esp_avrc_playback_stat_t lastPlayState = esp_avrc_playback_stat_t::ESP_AVRC_PLAYBACK_PLAYING;
esp_avrc_playback_stat_t playbackState = esp_avrc_playback_stat_t::ESP_AVRC_PLAYBACK_PLAYING;


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
  cycleFlag = true;
}
void recevieEvent(){
  byte msg = Wire.read();
  if(msg < 0x0b){selectedReg = msg;}
  else{toggleFlag = true;}
}
void requestEvent() {
  switch(selectedReg){
    case 0x02:
      Wire.write(playbackState);
      break;
    case 0x03:
      Wire.write(cycleFlag);
      break;
  }
}
void setup() {
  // put your setup code here, to run once:
  Wire.Begin(1); // change to 2 for second slave;
  Wire.onRequest(requestEvent);
  Wire.onReceive(recevieEvent);
  Serial.begin(115200);
  auto cfg = i2s.defaultConfig();
  cfg.pin_bck = 14;
  cfg.pin_ws = 12;
  cfg.pin_data = 13;
  i2s.begin(cfg);
  a2dp_sink.set_avrc_metadata_callback(avrc_metadata_callback);
  a2dp_sink.set_avrc_rn_track_change_callback(avrc_rn_track_change_callback);
  a2dp_sink.set_avrc_rn_playstatus_callback(avrc_rn_playstatus_callback);
}

void loop() {
  if(toggleFlag){
    if(playbackState == esp_avrc_playback_stat_t::ESP_AVRC_PLAYBACK_PLAYING ){a2dp_sink.pause();}
    else(a2dp_sink.play();)
  }
}
