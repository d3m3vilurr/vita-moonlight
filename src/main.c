/*
 * This file is part of Moonlight Embedded.
 *
 * Copyright (C) 2015, 2016 Iwan Timmer
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

#include "loop.h"
#include "client.h"
#include "connection.h"
#include "configuration.h"
#include "audio.h"
#include "video.h"
#include "discover.h"
#include "config.h"
#include "platform.h"
#include "sdl.h"

#include "input/evdev.h"
#include "input/udev.h"
#include "input/cec.h"
#include "input/sdlinput.h"
#include "input/vita.h"

#include <Limelight.h>

#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <openssl/rand.h>
#include <openssl/evp.h>
#include <ctype.h>

#include <psp2/net/net.h>
#include <psp2/sysmodule.h>
#include <psp2/appmgr.h>

#include <psp2/ctrl.h>
#include <psp2/touch.h>
#include <psp2/rtc.h>

#include "graphics.h"

CONFIGURATION config = {0};

static void applist(PSERVER_DATA server) {
  PAPP_LIST list = NULL;
  if (gs_applist(server, &list) != GS_OK) {
    fprintf(stderr, "Can't get app list\n");
    return;
  }

  for (int i = 1;list != NULL;i++) {
    printf("%d. %s\n", i, list->name);
    list = list->next;
  }
}

static int get_app_id(PSERVER_DATA server, const char *name) {
  PAPP_LIST list = NULL;
  if (gs_applist(server, &list) != GS_OK) {
    printf("Can't get app list\n");
    return -1;
  }

  while (list != NULL) {
    if (strcmp(list->name, name) == 0)
      return list->id;

    list = list->next;
  }
  return -1;
}

enum {
    SESSION_READY = 0,
    SESSION_CONNECTED = 1,
    SESSION_DISCONNECTED = 2,
};

static int session_state = SESSION_READY;

static void stream(PSERVER_DATA server, PCONFIGURATION config, enum platform system) {
  int appId = get_app_id(server, config->app);
  if (appId<0) {
    printf("Can't find app %s\n", config->app);
    loop_forever();
    exit(-1);
  }

  printf("Got configuration: width %d height %d fps %d bitrate %d\n", config->stream.width, config->stream.height, config->stream.fps, config->stream.bitrate);
  int ret = gs_start_app(server, &config->stream, appId, config->sops, config->localaudio);
  if (ret < 0) {
    if (ret == GS_NOT_SUPPORTED_4K)
      printf("Server doesn't support 4K\n");
    else
      printf("Errorcode starting app: %d\n", ret);

    loop_forever();
    exit(-1);
  }

  int drFlags = 0;
  if (config->fullscreen)
    drFlags |= DISPLAY_FULLSCREEN;

  if (config->forcehw)
    drFlags |= FORCE_HARDWARE_ACCELERATION;
  printf("Stream %d x %d, %d fps, %d kbps\n", config->stream.width, config->stream.height, config->stream.fps, config->stream.bitrate);
  LiStartConnection(server->address, &config->stream, &connection_callbacks, platform_get_video(system), platform_get_audio(system), NULL, drFlags, server->serverMajorVersion);
  session_state = SESSION_CONNECTED;
}

static void help() {
  printf("Usage: moonlight [action] (options) [host]\n");
  printf("       moonlight [configfile]\n");
  printf("\n Actions\n\n");
  printf("\tmap\t\t\tCreate mapping file for gamepad\n");
  printf("\tpair\t\t\tPair device with computer\n");
  printf("\tunpair\t\t\tUnpair device with computer\n");
  printf("\tstream\t\t\tStream computer to device\n");
  printf("\tlist\t\t\tList available games and applications\n");
  printf("\tquit\t\t\tQuit the application or game being streamed\n");
  printf("\thelp\t\t\tShow this help\n");
  printf("\n Global Options\n\n");
  printf("\t-config <config>\tLoad configuration file\n");
  printf("\t-save <config>\t\tSave configuration file\n");
  printf("\n Streaming options\n\n");
  printf("\t-720\t\t\tUse 1280x720 resolution [default]\n");
  printf("\t-1080\t\t\tUse 1920x1080 resolution\n");
  printf("\t-width <width>\t\tHorizontal resolution (default 1280)\n");
  printf("\t-height <height>\tVertical resolution (default 720)\n");
  printf("\t-30fps\t\t\tUse 30fps\n");
  printf("\t-60fps\t\t\tUse 60fps [default]\n");
  printf("\t-bitrate <bitrate>\tSpecify the bitrate in Kbps\n");
  printf("\t-packetsize <size>\tSpecify the maximum packetsize in bytes\n");
  printf("\t-forcehevc\t\tUse high efficiency video decoding (HEVC)\n");
  printf("\t-remote\t\t\tEnable remote optimizations\n");
  printf("\t-app <app>\t\tName of app to stream\n");
  printf("\t-nosops\t\t\tDon't allow GFE to modify game settings\n");
  printf("\t-localaudio\t\tPlay audio locally\n");
  printf("\t-surround\t\tStream 5.1 surround sound (requires GFE 2.7)\n");
  printf("\t-keydir <directory>\tLoad encryption keys from directory\n");
  #ifdef HAVE_SDL
  printf("\n Video options (SDL Only)\n\n");
  printf("\t-windowed\t\tDisplay screen in a window\n");
  #endif
  #ifdef HAVE_EMBEDDED
  printf("\n I/O options\n\n");
  printf("\t-mapping <file>\t\tUse <file> as gamepad mapping configuration file (use before -input)\n");
  printf("\t-input <device>\t\tUse <device> as input. Can be used multiple times\n");
  printf("\t-audio <device>\t\tUse <device> as audio output device\n");
  printf("\t-forcehw \t\tTry to use video hardware acceleration\n");
  #endif
  printf("\nUse Ctrl+Alt+Shift+Q to exit streaming session\n\n");
  exit(0);
}

static void pair_check(PSERVER_DATA server) {
  if (!server->paired) {
    fprintf(stderr, "You must pair with the PC first\n");
    exit(-1);
  }
}

static void vita_init() {
  // Seed OpenSSL with Sony-grade random number generator
  char random_seed[0x40] = {0};
  sceKernelGetRandomNumber(random_seed, sizeof(random_seed));
  RAND_seed(random_seed, sizeof(random_seed));
  OpenSSL_add_all_algorithms();

  // This is only used for PIN codes, doesn't really matter
  srand(time(NULL));

  printf("Moonlight Embedded %d.%d.%d (%s)\n", VERSION_MAJOR, VERSION_MINOR, VERSION_PATCH, COMPILE_OPTIONS);

  int ret = 0;

  ret = sceSysmoduleLoadModule(SCE_SYSMODULE_NET);

  size_t net_mem_sz = 100 * 1024;
  SceNetInitParam net_param = {0};
  net_param.memory = calloc(net_mem_sz, 1);
  net_param.size = net_mem_sz;
  ret = sceNetInit(&net_param);

  ret = sceNetCtlInit();
  // TODO(xyz): cURL breaks when socket FD is too big, very hacky workaround below!
  int s = sceNetSocket("", SCE_NET_AF_INET, SCE_NET_SOCK_STREAM, 0);
  sceNetSocketClose(s);
  if (s >= 20) {
    printf("Cycling sockets...\n");
    int c = 0;
    do {
      c = sceNetSocket("", SCE_NET_AF_INET, SCE_NET_SOCK_STREAM, 0);
      sceNetSocketClose(c);
    } while (c >= 5);
  }
}

SceAppMgrSystemEvent event;

void loop_forever(void) {
  while (connection_is_ready()) {
    sceAppMgrReceiveSystemEvent(&event);
    if (session_state == SESSION_CONNECTED && event.systemEvent == SCE_APPMGR_SYSTEMEVENT_ON_RESUME) {
      vitainput_stop();
      vitapower_stop();
      vitavideo_off();
      LiStopConnection();
      break;
    }
    sceKernelDelayThread(100 * 1000);
  }
  session_state = SESSION_DISCONNECTED;
}

static unsigned buttons[] = {
  SCE_CTRL_SELECT,
  SCE_CTRL_START,
  SCE_CTRL_UP,
  SCE_CTRL_RIGHT,
  SCE_CTRL_DOWN,
  SCE_CTRL_LEFT,
  SCE_CTRL_LTRIGGER,
  SCE_CTRL_RTRIGGER,
  SCE_CTRL_TRIANGLE,
  SCE_CTRL_CIRCLE,
  SCE_CTRL_CROSS,
  SCE_CTRL_SQUARE,
};

static int get_key(void) {
  static unsigned prev = 0;
  SceCtrlData pad;
  while (1) {
    memset(&pad, 0, sizeof(pad));
    sceCtrlPeekBufferPositive(0, &pad, 1);
    unsigned new = prev ^ (pad.buttons & prev);
    prev = pad.buttons;
    for (int i = 0; i < sizeof(buttons)/sizeof(*buttons); ++i)
      if (new & buttons[i])
        return buttons[i];

    sceKernelDelayThread(1000); // 1ms
  }
}

static void vita_pair(SERVER_DATA *server) {
  char pin[5];
  sprintf(pin, "%d%d%d%d", (int)rand() % 10, (int)rand() % 10, (int)rand() % 10, (int)rand() % 10);
  psvDebugScreenSetFgColor(COLOR_BLACK);
  psvDebugScreenSetBgColor(COLOR_WHITE);
  printf("Please enter the following PIN on the target PC: %s\n", pin);
  psvDebugScreenSetFgColor(COLOR_WHITE);
  psvDebugScreenSetBgColor(COLOR_BLACK);
  int ret = gs_pair(server, &pin[0]);
  if (ret == 0) {
    psvDebugScreenSetFgColor(COLOR_GREEN);
    printf("Paired successfully\n");
    psvDebugScreenSetFgColor(COLOR_WHITE);
  } else {
    printf("Error pairing: 0x%x %s\n", ret, gs_error);
  }
}

static SERVER_DATA server;

int session_init() {
  int ret;
  if ((ret = gs_init(&server, config.key_dir)) == GS_OUT_OF_MEMORY) {
    printf("Not enough memory\n");
    return ret;
  } else if (ret == GS_INVALID) {
    printf("Invalid data received from server: %s\n", config.address, gs_error);
    return ret;
  } else if (ret == GS_UNSUPPORTED_VERSION) {
    if (!config.unsupported_version) {
      printf("Unsupported version: %s\n", gs_error);
      return ret;
    }
  } else if (ret != GS_OK) {
    printf("Can't connect to server %s\n", config.address);
    return ret;
  }
  return ret;
}

int session_reset() {
  gs_quit_app(&server);
  session_init(&server);
}

int main(int argc, char* argv[]) {
  int ret = 0;

  psvDebugScreenInit();
  vita_init();

  printf("Attempting to parse config\n");
  config_parse(argc, argv, &config);
  config.platform = "vita";
  strcpy(config.key_dir, "ux0:data/moonlight/");

  if (!vitainput_init(config)) {
    loop_forever();
  }
  if (!vitapower_init(config)) {
    loop_forever();
  }
  if (!vitavideo_init(config)) {
    loop_forever();
  }

  server.address = malloc(sizeof(char)*256);
  strcpy(server.address, config.address);

  psvDebugScreenSetFgColor(COLOR_GREEN);
  printf("Server address: %s\n", server.address);
  psvDebugScreenSetFgColor(COLOR_WHITE);
  enum platform system = VITA;

  if (session_init() != GS_OK) {
    loop_forever();
  }

  printf("NVIDIA %s, GFE %s (protocol version %d)\n", server.gpuType, server.gfeVersion, server.serverMajorVersion);
  printf("\n");

again:
  connection_state_reset();

  if (session_state == SESSION_DISCONNECTED) {
    if (session_init() != GS_OK) {
      loop_forever();
    }
  }

  printf("Press X to pair (You need to do it once)\n");
  printf("Press O to launch steam\n");

  switch(get_key()) {
  case SCE_CTRL_CROSS:
    vita_pair(&server);
    break;
  case SCE_CTRL_CIRCLE:
    sceAppMgrReceiveSystemEvent(&event);
    if (event.systemEvent == SCE_APPMGR_SYSTEMEVENT_ON_RESUME) {
      session_reset();
    }
    stream(&server, &config, system);
    loop_forever();
    break;
  }
  goto again;
}
