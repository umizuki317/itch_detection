/*
 *  step_counter.ino - Step Conter example application
 *  Copyright 2018 Sony Semiconductor Solutions Corporation
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
//https://developer.sony.com/develop/spresense/docs/arduino_tutorials_ja.html#_step_counter_%E3%81%AE%E7%89%A9%E7%90%86%E3%82%BB%E3%83%B3%E3%82%B5%E3%82%92%E5%A4%89%E6%9B%B4%E3%81%97%E3%81%A6%E3%81%BF%E3%82%8B
//#include <BMI160Gen.h>
#include <stdio.h>
#include <MP.h>

#include <Wire.h>
#include "KX122.h"
KX122 kx122(KX122_DEVICE_ADDRESS_1F);

#ifndef SUBCORE
#error "Core selection is wrong!!"
#endif
#define SUBID "Sub2"
/* Const values */
#define MY_MSGID    11
#define PI 3.141592

struct MyPacket {
  volatile int status; /* 0:ready, 1:busy */
  int message;
};

float raw_d[50];
float sin_d[50];
float tmpx,tmpy;
float mean_x,mean_y;
int count = 0;
MyPacket packet;


void setup() {
  byte rc;
  int ret = 0;
  tmpx = 0.0;
  tmpy = 0.0;
  ret = MP.begin();
  if (ret < 0) {
    errorLoop(2);
  }
  Serial.begin(115200);
  while (!Serial);

  Wire.begin();

  rc = kx122.init();
  if (rc != 0) {
    Serial.println(F("KX122 initialization failed"));
    Serial.flush();
  }
  mean_x = 0.0;
  mean_y = 0.0;
  for(int k=0;k<50;k++){
    byte rc;
    float acc[3];
    unsigned long prev = millis();
    rc = kx122.get_val(acc);
    mean_x += acc[0];
    mean_y += acc[1];
    while(millis()-prev<20);
  }
  mean_x = mean_x/50.0;
  mean_y = mean_y/50.0;
  for(int i = 0;i<50;i++){
    sin_d[i] = sin(i*4*PI/50);
  }
}

void loop() {
  byte rc;
  float acc[3];
  unsigned long prev = millis();

  rc = kx122.get_val(acc);
  if (rc == 0) {
    raw_d[count] = acc[0]-mean_x+acc[1]-mean_y;
    tmpx += acc[0];
    tmpy += acc[1];
  }
  count++;
  if(count==50){
    float thread;
    for(int k = 0;k<50;k++){
      thread += sin_d[k]*raw_d[k];
    }
    if(thread>9.0){
      int ret;
      if(packet.status == 0){
      packet.status = 1;
      packet.message = 1;
      ret = MP.Send(MY_MSGID, &packet);
      if (ret < 0) {
        printf("MP.Send error = %d\n", ret);
        }
      //packet.message = 0;
      }
    }
    printf("%f",thread);
    printf("\n");
    mean_x = tmpx/50.0;
    mean_y = tmpy/50.0;
  }
  count %= 50;
  while(millis()-prev<20);
}

void errorLoop(int num)
{
  int i;
  while (1) {
    for (i = 0; i < num; i++) {
      ledOn(LED0);
      delay(300);
      ledOff(LED0);
      delay(300);
    }
    delay(1000);
  }
}
