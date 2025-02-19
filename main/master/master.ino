#include "AudioTools.h"
#include "BluetoothA2DPSink.h"
#include <Wire.h>

I2SStream i2s;
BluetoothA2DPSink a2dp_sink(i2s);

enum connection_state {WAITING = 0x01 ,PAIRING,CONNECTED};

int selectorPinA = 26;
int selectorPinB = 25;

int CurrentCycle = 0;
int NextCycle = 1;
int PrevCycle = 2;
esp_avrc_playback_stat_t playbackStates[] = {0,0,0};
connection_state connectionStates[] = {0,0,0};
bool triMode = false;
uint8_t track_change_flag = 0;
bool cycleFlag = false;
bool globalPlayback = true;
bool lastGlobalPlayback = true;
int connectTimeout = 60;
connection_state connectionState = PAIRING;
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


void toggle(int cycleid){
  if(cycleid == 0){
    if(playbackState == esp_avrc_playback_stat_t::ESP_AVRC_PLAYBACK_PLAYING ){a2dp_sink.pause();}
    else(a2dp_sink.play();)
  }
  else{
    Wire.beginTransmission(cycleid);
    if(states[cycleid] == esp_avrc_playback_stat_t::ESP_AVRC_PLAYBACK_PLAYING){Wire.write(0x0a);}//send pause signal
    else{Wire.write(0x0b);}//send play signal
    Wire.endTransmission();
  }
}
void cycle(){
  PrevCycle = CurrentCycle;
  if (CurrentCycle < 2){CurrentCycle++;}
  else if (CurrentCycle == 2){CurrentCycle = 0;}
  else{CurrentCycle = 0;NextCycle = CurrentCycle+1;}
  if(CurrentCycle == 2){NextCycle = 0;}
  else{NextCycle = CurrentCycle+1;}
  cycleFlag=false;
}
//basic for now, could be more advanced with more playback states.
void refresh() {
  Serial.println("refreshing " + String(CurrentCycle));
  if(states[CurrentCycle] != esp_avrc_playback_stat_t::ESP_AVRC_PLAYBACK_PLAYING){
    toggle(CurrentCycle);
    //timeoutCheck(CurrentCycle,true);
  }
  if(states[NextCycle] == esp_avrc_playback_stat_t::ESP_AVRC_PLAYBACK_PLAYING){
	  toggle(NextCycle);
    //timeoutCheck(NextCycle,false);
  }
  if(states[PrevCycle] == esp_avrc_playback_stat_t::ESP_AVRC_PLAYBACK_PLAYING){
	  toggle(PrevCycle);
    //timeoutCheck(PrevCycle,false);
  }
}

void changeRequestReg(int id, int reg,bool stop){
  Wire.beginTransmission(id);
  Wire.write(reg);
  Wire.endTransmission(stop);
}
//state1 nothing, state2 cycle, state3 refersh.
int readState() {
  // playback state of master
  states[0] = playbackState;
  //get playback state of slave 1
  changeRequestReg(1,2,false);
  Wire.requestFrom(1,1);
  states[1] = Wire.read();
  Wire.endTransmission();
  //get cycleflag from slave 1
  changeRequestReg(1,3,false);
  Wire.requestFrom(1,1)
  cycleFlag = cycleFlag || Wire.read() 
  Wire.endTransmission();
  //get data for slave 2 if enabled
  if(triMode){
    changeRequestReg(2,2,false);
    Wire.requestFrom(2,1);
    states[2] = Wire.read();
    Wire.endTransmission();
    //get cycleflag from slave 1
    changeRequestReg(2,3,false);
    Wire.requestFrom(2,1)
    cycleFlag = cycleFlag || Wire.read() 
    Wire.endTransmission();
  } 
  // if paused or play button is triggered toggle the current device
  if(globalPlayback != lastGlobalPlayback){
    toggle(CurrentCycle);
    Serial.println("Paused = "+String(globalPlayback));
    return 1;
  }
  int playing = 0;
  for(int i = 0; i<3;i++){
    if(states[i] == esp_avrc_playback_stat_t::ESP_AVRC_PLAYBACK_PLAYING){playing++;}
    Serial.println("device " + String(i) + " is " + String(states[i]));
  }
  if(cycleFlag && globalPlayback){return 2;}
  if(playing == 0 && globalPlayback){
  	return 3;
  }
  else if(playing > 1){
  	return 3;
  }
  else{return 1;}
}
void setup() {
  pinMode(selectorPinA,OUTPUT);
  pinMode(selectorPinB,OUTPUT);
  Wire.Begin();
  Serial.begin(115200);
  auto cfg = i2s.defaultConfig();
  cfg.pin_bck = 14;
  cfg.pin_ws = 12;
  cfg.pin_data = 13;
  i2s.begin(cfg);
  a2dp_sink.set_avrc_metadata_callback(avrc_metadata_callback);
  a2dp_sink.set_avrc_rn_track_change_callback(avrc_rn_track_change_callback);
  a2dp_sink.set_avrc_rn_playstatus_callback(avrc_rn_playstatus_callback);
  a2dp_sink.start("BTCycler1");
  while(!a2dp_sink.is_connected()) {delay(1000);}
  connectionState = CONNECTED;
  Wire.beginTransmission(1);
  Wire.write(0x00);
  Wire.endTransmission();
  changeRequestReg(1,1,true);
  for(int i =0; i<connectTimeout;i++){
    byte connectState = Wire.requestfrom(1,1);
    if(connectState == connection_state::CONNECTED){
      Wire.endTrasmission();
      connectionStates[1] = connection_state::CONNECTED;
      break;
    }
    delay(1000);
  }
  if(connectionStates[1] = connection_state::CONNECTED){
    Wire.beginTransmission(2);
    Wire.write(0x01);
    Wire.endTransmission();
    changeRequestReg(2,1,true);
    for(int i =0; i<connectTimeout;i++){
      byte connectState = Wire.requestfrom(1,1);
      if(connectState == connection_state::CONNECTED){
        Wire.endTrasmission();
        connectionStates[2] = connection_state::CONNECTED;
        triMode = true;
        break;
      }
      delay(1000);
    }
  }

}
void loop() {
  int path = readState();
  if(path == 2){
    toggle(CurrentCycle);
    toggle(NextCycle);
    cycle();
  }
  else if(path == 3){refresh();}
  lastPlayState = playbackState;
  lastGlobalPlayback = globalPlayback;
}