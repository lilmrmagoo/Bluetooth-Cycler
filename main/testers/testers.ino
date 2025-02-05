#include "AudioTools.h"
#include "BluetoothA2DPSink.h"
#include <Wire.h>

const uint8_t I2S_SCK = 5;       /* Audio data bit clock */
const uint8_t I2S_WS = 25;       /* Audio data left and right clock */
const uint8_t I2S_SDOUT = 26;    /* ESP32 audio data output (to speakers) */
I2SStream i2s;
BluetoothA2DPSink a2dp_sink(i2s);

esp_avrc_playback_stat_t playbackState = esp_avrc_playback_stat_t::ESP_AVRC_PLAYBACK_PLAYING;
uint8_t track_change_flag = 0;
bool cycleFlag = false;
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

void setup(){
  Serial.begin(115200);
  a2dp_sink.set_avrc_metadata_callback(avrc_metadata_callback);
  a2dp_sink.set_avrc_rn_track_change_callback(avrc_rn_track_change_callback);
  a2dp_sink.set_avrc_rn_playstatus_callback(avrc_rn_playstatus_callback);
  a2dp_sink.start("BTCycler2");
}

void loop(){
}