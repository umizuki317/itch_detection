/*
 *  MainAudio.ino - MP Example for Audio FFT 
 *  Copyright 2019 Sony Semiconductor Solutions Corporation
 *
 *  This library is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU Lesser General Public
 *  License as published by the Free Software Foundation; either
 *  version 2.1 of the License, or (at your option) any later version.
 *
 *  This library is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *  Lesser General Public License for more details.
 *
 *  You should have received a copy of the GNU Lesser General Public
 *  License along with this library; if not, write to the Free Software
 *  Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA
 */

#ifdef SUBCORE
#error "Core selection is wrong!!"
#endif

#include <MP.h>
#include <Audio.h>
#include "ESP8266.h"

#define SSID        "your SSID"
#define PASSWORD    "Your PASS"
#define HOST_NAME   "YOUR NAME"
#define HOST_PORT   YOUR PORT
ESP8266 wifi;


AudioClass *theAudio;
//AudioClass *theAudio_beep;

struct MyPacket {
  volatile int status; /* 0:ready, 1:busy */
  int message = 1;
};


/* Select mic channel number */
//const int mic_channel_num = 1;
//const int mic_channel_num = 2;
const int mic_channel_num = 1;

const int subcore1 = 1;
const int subcore2 = 2;

//const int subcore2 = 2;
int ret1=0;
int ret2=0;

struct Capture {
  void *buff;
  int  sample;
  int  chnum;
};


void setup()
{
  int ret;
  //begin setup
  Serial.begin(115200);
  while (!Serial);
  Serial2.begin(115200);
  
  wifi.begin(Serial2, 115200);
  wifi.setOprToStationSoftAP();
  wifi.joinAP(SSID, PASSWORD);
  wifi.disableMUX();
  Serial.println("Init Audio Library");
  theAudio = AudioClass::getInstance();
  theAudio->begin();

  Serial.println("Init Audio Recorder");
  /* Select input device as AMIC */
  //theAudio->setRecorderMode(AS_SETRECDR_STS_INPUTDEVICE_MIC, 210);
  theAudio->setRecorderMode(AS_SETRECDR_STS_INPUTDEVICE_MIC);
  
  /* Set PCM capture */
  uint8_t channel;
  switch (mic_channel_num) {
  case 1: channel = AS_CHANNEL_MONO;   break;
  case 2: channel = AS_CHANNEL_STEREO; break;
  case 4: channel = AS_CHANNEL_4CH;    break;
  }
  theAudio->initRecorder(AS_CODECTYPE_PCM, "/mnt/sd0/BIN", AS_SAMPLINGRATE_48000, channel);

  /* Launch SubCore */
  for (int subcore = 1; subcore <= 2; ++subcore) {
    MP.begin(subcore);
  }
  MP.RecvTimeout(MP_RECV_POLLING);
  Serial.println("Rec start!");
  theAudio->startRecorder();
}

void loop()
{
  int8_t   sndid = 100; /* user-defined msgid */
  Capture  capture;

  static const int32_t buffer_sample = 768 * mic_channel_num;
  static const int32_t buffer_size = buffer_sample * sizeof(int16_t);
  static char  buffer[buffer_size];
  uint32_t read_size;
  int8_t msgid1;
  MyPacket *packet1;
  int8_t msgid2;
  MyPacket *packet2;
  
  /* Read frames to record in buffer */
  int err = theAudio->readFrames(buffer, buffer_size, &read_size);

  if (err != AUDIOLIB_ECODE_OK && err != AUDIOLIB_ECODE_INSUFFICIENT_BUFFER_AREA) {
    printf("Error err = %d\n", err);
    sleep(1);
    theAudio->stopRecorder();
    exit(1);
  }

  if ((read_size != 0) && (read_size == buffer_size)) {
    capture.buff   = buffer;
    capture.sample = buffer_sample / mic_channel_num;
    capture.chnum  = mic_channel_num;
    MP.Send(sndid, &capture, subcore1);
  } else {
    usleep(1);
  }

  ret1 = MP.Recv(&msgid1,&packet1,subcore1);
  if(packet1->message ==1){
    MP.RecvTimeout(1000);
    ret2 = MP.Recv(&msgid2,&packet2,subcore2);
    MP.RecvTimeout(MP_RECV_POLLING);
    //printf("%d\n", packet->message);
    if(packet2->message == 1){
      theAudio->stopRecorder();
      theAudio->setReadyMode();
      
      theAudio->setPlayerMode(AS_SETPLAYER_OUTPUTDEVICE_SPHP, 0, 0);
      theAudio->setBeep(1,0,150);
      usleep(5000 * 1000);
      theAudio->setBeep(0,0,0);
     
      theAudio->setReadyMode();
      theAudio->setRecorderMode(AS_SETRECDR_STS_INPUTDEVICE_MIC);
      theAudio->initRecorder(AS_CODECTYPE_PCM, "/mnt/sd0/BIN", AS_SAMPLINGRATE_48000, AS_CHANNEL_MONO);
      theAudio->startRecorder();
      packet1->message = 0;
      packet2->message = 0;
      
    }
      /* status -> ready */
      packet1->status = 0;
      packet2->status = 0;
    }
}
