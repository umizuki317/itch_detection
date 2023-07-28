/*
 *  SubFFT.ino - MP Example for Audio FFT 
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
#include <stdio.h>
#include <MP.h>

#ifndef SUBCORE
#error "Core selection is wrong!!"
#endif
#define SUBID "Sub1"

#define MY_MSGID    10

struct MyPacket {
  volatile int status; /* 0:ready, 1:busy */
  int message;
};

/* Use CMSIS library */
#define ARM_MATH_CM4
#define __FPU_PRESENT 1U
#include <arm_math.h>

#include "RingBuff.h"

/* Select FFT length */

//#define FFTLEN 32
//#define FFTLEN 64
//#define FFTLEN 128
//#define FFTLEN 256
//#define FFTLEN 512
#define FFTLEN 1024
//#define FFTLEN 2048
//#define FFTLEN 4096
MyPacket packet;
const int g_channel = 4;

/* Ring buffer */

#define INPUT_BUFFER (1024 * 4)
RingBuff ringbuf[g_channel] = {
  RingBuff(INPUT_BUFFER),
  RingBuff(INPUT_BUFFER),
  RingBuff(INPUT_BUFFER),
  RingBuff(INPUT_BUFFER)
};

/* Allocate the larger heap size than default */

USER_HEAP_SIZE(64 * 1024);

/* Temporary buffer */

float pSrc[FFTLEN];
float pDst[FFTLEN];
float tmpBuf[FFTLEN];

/* MultiCore definitions */

struct Capture {
  void *buff;
  int  sample;
  int  chnum;
};

void setup()
{
  int ret = 0;
  //memset(&packet, 0, sizeof(packet));
  /* Initialize MP library */
  ret = MP.begin();
  if (ret < 0) {
    errorLoop(2);
  }
  /* receive with non-blocking */
  MP.RecvTimeout(MP_RECV_POLLING);
}

void loop()
{
  int      ret;
  int8_t   msgid;
  Capture *capture;

  /* Receive PCM captured buffer from MainCore */
  ret = MP.Recv(&msgid, &capture);
  if (ret >= 0) {
    if (capture->chnum == 1) {
      /* the faster optimization */
      ringbuf[0].put((q15_t*)capture->buff, capture->sample);
    } else {
      int i;
      for (i = 0; i < capture->chnum; i++) {
        ringbuf[i].put((q15_t*)capture->buff, capture->sample, capture->chnum, i);
      }
    }
  }

  while (ringbuf[0].stored() >= FFTLEN) {
    fft_processing(capture->chnum);
  }
}

void fft_processing(int chnum)
{
  int i;
  float peakFs[1] = {0.0f};
  float tmp;
  
  ringbuf[0].get(pSrc,FFTLEN);
  fft(pSrc, pDst, FFTLEN);
  peakFs[0] = get_peak_frequency(pDst,FFTLEN,&tmp);
  
  float thread = 95.0;
  if((tmp>thread) && (peakFs[0]>50.0) && (peakFs[0]<880.0)){

    int ret;
    static int count = 0;
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
}

void fft(float *pSrc, float *pDst, int fftLen)
{
  arm_rfft_fast_instance_f32 S;

#if (FFTLEN == 32)
  arm_rfft_32_fast_init_f32(&S);
#elif (FFTLEN == 64)
  arm_rfft_64_fast_init_f32(&S);
#elif (FFTLEN == 128)
  arm_rfft_128_fast_init_f32(&S);
#elif (FFTLEN == 256)
  arm_rfft_256_fast_init_f32(&S);
#elif (FFTLEN == 512)
  arm_rfft_512_fast_init_f32(&S);
#elif (FFTLEN == 1024)
  arm_rfft_1024_fast_init_f32(&S);
#elif (FFTLEN == 2048)
  arm_rfft_2048_fast_init_f32(&S);
#elif (FFTLEN == 4096)
  arm_rfft_4096_fast_init_f32(&S);
#endif

  /* calculation */
  arm_rfft_fast_f32(&S, pSrc, tmpBuf, 0);

  arm_cmplx_mag_f32(&tmpBuf[2], &pDst[1], fftLen / 2 - 1);
  pDst[0] = tmpBuf[0];
  pDst[fftLen / 2] = tmpBuf[1];
}

float get_peak_frequency(float *pData, int fftLen,float* value)
{
  float g_fs = 48000.0f;
  uint32_t index;
  float maxValue;
  float delta;
  float peakFs;

  arm_max_f32(pData, fftLen / 2, &maxValue, &index);

  delta = 0.5 * (pData[index - 1] - pData[index + 1])
    / (pData[index - 1] + pData[index + 1] - (2.0f * pData[index]));
  peakFs = (index + delta) * g_fs / (fftLen - 1);
  //float tmp_index = (float)index;
  *value = pData[index];
  
  return peakFs;
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
