/*
 * This file is part of Moonlight Embedded.
 *
 * Copyright (C) 2015 Iwan Timmer
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

#include "connection.h"
#include "global.h"
#include "config.h"
#include "power/vita.h"
#include "input/vita.h"
#include "video/vita.h"

#include <stdio.h>
#include <stdbool.h>

#include "graphics.h"
static int connection_status = LI_READY;

void connection_connection_started()
{
  connection_status = LI_CONNECTED;
  vitainput_start();
  vitapower_start();
  vitavideo_on();
}

void connection_connection_terminated()
{
  vitainput_stop();
  vitapower_stop();
  vitavideo_off();
  printf("connection terminated\n");
  connection_status = LI_DISCONNECTED;
  quit();
}

void connection_display_message(char *msg)
{
  printf("%s\n", msg);
}

void connection_display_transient_message(char *msg)
{
  printf("%s\n", msg);
}

void connection_state_reset() {
  connection_status = LI_READY;
}

bool connection_is_ready() {
  return connection_status != LI_DISCONNECTED;
}

CONNECTION_LISTENER_CALLBACKS connection_callbacks = {
  .stageStarting = NULL,
  .stageComplete = NULL,
  .stageFailed = NULL,
  .connectionStarted = connection_connection_started,
  .connectionTerminated = connection_connection_terminated,
  .displayMessage = connection_display_message,
  .displayTransientMessage = connection_display_transient_message,
};
