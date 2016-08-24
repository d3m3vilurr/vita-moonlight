/*
 * This file is part of Moonlight Embedded.
 *
 * Copyright (C) 2016 Ilya Zhuravlev
 *
 * Moonlight is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 3 of the License, or
 * (at your option) any later version.
 *
 * Moonlight is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with Moonlight; if not, see <http://www.gnu.org/licenses/>.
 */

#include "../video.h"
#include "sps.h"

#include <Limelight.h>

#include <stdbool.h>
#include <psp2/kernel/sysmem.h>
#include <psp2/display.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "videodec.h"

#define DECODER_BUFFER_SIZE 92*1024

static char* ffmpeg_buffer;

enum {
  SCREEN_WIDTH = 960,
  SCREEN_HEIGHT = 544,
  LINE_SIZE = 960,
  FRAMEBUFFER_SIZE = 2 * 1024 * 1024,
  FRAMEBUFFER_ALIGNMENT = 256 * 1024
};
int backbuffer;
void *framebuffer[2];

SceAvcdecCtrl decoder = {0};

static FILE* fd;
static const char* fileName = "ux0:data/moonlight/fake.h264";

static void vita_setup(int videoFormat, int width, int height, int redrawRate, void* context, int drFlags) {
  gs_sps_init();

  printf("vita video setup\n");
  fd = fopen(fileName, "w");

  SceKernelAllocMemBlockOpt opt = { 0 };
  opt.size = sizeof(opt);
  opt.attr = 0x00000004;
  opt.alignment = FRAMEBUFFER_ALIGNMENT;
  SceUID displayblock = sceKernelAllocMemBlock("display", SCE_KERNEL_MEMBLOCK_TYPE_USER_CDRAM_RW, 2 * FRAMEBUFFER_SIZE, &opt);
  printf("displayblock: 0x%08x\n", displayblock);
  void *base;
  sceKernelGetMemBlockBase(displayblock, &base);
  printf("base: 0x%08x\n", base);

  framebuffer[0] = base;
  framebuffer[1] = (char*)base + FRAMEBUFFER_SIZE;
  backbuffer = 1;

  SceDisplayFrameBuf framebuf = { 0 };
  framebuf.size = sizeof(framebuf);
  framebuf.base = base;
  framebuf.pitch = SCREEN_WIDTH;
  framebuf.pixelformat = SCE_DISPLAY_PIXELFORMAT_A8B8G8R8;
  framebuf.width = SCREEN_WIDTH;
  framebuf.height = SCREEN_HEIGHT;
  int ret = sceDisplaySetFrameBuf(&framebuf, 1);
  printf("SetFrameBuf: 0x%x\n", ret);

  ffmpeg_buffer = malloc(DECODER_BUFFER_SIZE);
  if (ffmpeg_buffer == NULL) {
    printf("Not enough memory\n");
    exit(1);
  }

  SceVideodecQueryInitInfoHwAvcdec init = {0};
  init.size = sizeof(init);
  init.horizontal = width;
  init.vertical = height;
  init.numOfRefFrames = 5;
  init.numOfStreams = 1;

  SceAvcdecQueryDecoderInfo decoder_info = {0};
  decoder_info.horizontal = init.horizontal;
  decoder_info.vertical = init.vertical;
  decoder_info.numOfRefFrames = init.numOfRefFrames;

  SceAvcdecDecoderInfo decoder_info_out = {0};

  ret = sceVideodecInitLibrary(0x1001, &init);
  printf("sceVideodecInitLibrary 0x%x\n", ret);
  ret = sceAvcdecQueryDecoderMemSize(0x1001, &decoder_info, &decoder_info_out);
  printf("sceAvcdecQueryDecoderMemSize 0x%x size 0x%x\n", ret, decoder_info_out.frameMemSize);

  size_t sz = (decoder_info_out.frameMemSize + 0xFFFFF) & ~0xFFFFF;
  decoder.frameBuf.size = sz;
  printf("allocating size 0x%x\n", sz);
  int decoderblock = sceKernelAllocMemBlock("decoder", SCE_KERNEL_MEMBLOCK_TYPE_USER_MAIN_PHYCONT_NC_RW, sz, NULL);
  printf("decoderblock: 0x%08x\n", decoderblock);
  sceKernelGetMemBlockBase(decoderblock, &decoder.frameBuf.pBuf);
  printf("base: 0x%08x\n", decoder.frameBuf.pBuf);

  ret = sceAvcdecCreateDecoder(0x1001, &decoder, &decoder_info);
  printf("sceAvcdecCreateDecoder 0x%x\n", ret);
}

static void vita_cleanup() {
  fclose(fd);
  #warning TODO cleanup
}

static unsigned numframes;

static int vita_submit_decode_unit(PDECODE_UNIT decodeUnit) {
  #if 0
  PLENTRY entry = decodeUnit->bufferList;
  while (entry != NULL) {
    fwrite(entry->data, entry->length, 1, fd);
    entry = entry->next;
  }
  #endif

  SceAvcdecAu au = {0};
  SceAvcdecArrayPicture array_picture = {0};
  SceAvcdecPicture picture = {0};
  SceAvcdecPicture *pictures = { &picture };
  array_picture.numOfElm = 1;
  array_picture.pPicture = &pictures;

  picture.size = sizeof(picture);
  picture.frame.pixelType = 0;
  picture.frame.framePitch = LINE_SIZE;
  picture.frame.frameWidth = SCREEN_WIDTH;
  picture.frame.frameHeight = SCREEN_HEIGHT;
  picture.frame.pPicture[0] = framebuffer[backbuffer];
  
  if (decodeUnit->fullLength < DECODER_BUFFER_SIZE) {
    PLENTRY entry = gs_sps_fix(&decodeUnit->bufferList, 0);
    // PLENTRY entry = decodeUnit->bufferList;
    int length = 0;
    while (entry != NULL) {
      memcpy(ffmpeg_buffer+length, entry->data, entry->length);
      length += entry->length;
      entry = entry->next;
    }
    au.es.pBuf = ffmpeg_buffer;
    au.es.size = decodeUnit->fullLength;
    au.dts.lower = 0xFFFFFFFF;
    au.dts.upper = 0xFFFFFFFF;
    au.pts.lower = 0xFFFFFFFF;
    au.pts.upper = 0xFFFFFFFF;

    int ret = 0;
    ret = sceAvcdecDecode(&decoder, &au, &array_picture);
    if (ret < 0)
      printf("sceAvcdecDecode (len=0x%x): 0x%x numOfOutput %d\n", decodeUnit->fullLength, ret, array_picture.numOfOutput);
    if (ret < 0) {
      return DR_NEED_IDR;
    }

    if (array_picture.numOfOutput == 1) {
#if 0
      static uint64_t prev_frame;
      uint64_t cur_frame;
      sceRtcGetCurrentTick(&cur_frame);
      sceClibPrintf("got frame in %d us\n", (int)(cur_frame - prev_frame));
      prev_frame = cur_frame;
#endif

      SceDisplayFrameBuf framebuf = { 0 };
      framebuf.size = sizeof(framebuf);
      framebuf.base = framebuffer[backbuffer];
      framebuf.pitch = SCREEN_WIDTH;
      framebuf.pixelformat = SCE_DISPLAY_PIXELFORMAT_A8B8G8R8;
      framebuf.width = SCREEN_WIDTH;
      framebuf.height = SCREEN_HEIGHT;
      int ret = sceDisplaySetFrameBuf(&framebuf, 1);
      backbuffer = (backbuffer + 1) % 2;
      if (ret < 0)
        printf("Failed to sceDisplaySetFrameBuf: 0x%x\n", ret);
    }
  } else {
    printf("Video decode buffer too small");
    exit(1);
  }

  // if (numframes++ % 6 == 0)
  //   return DR_NEED_IDR;

  return DR_OK;
}

DECODER_RENDERER_CALLBACKS decoder_callbacks_vita = {
  .setup = vita_setup,
  .cleanup = vita_cleanup,
  .submitDecodeUnit = vita_submit_decode_unit,
  .capabilities = CAPABILITY_DIRECT_SUBMIT,
};
