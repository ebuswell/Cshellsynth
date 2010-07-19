/** @file controller.h
 *
 * Controller
 *
 * Ruby version: @c Controllers::Controller
 */
/*
 * Copyright 2010 Evan Buswell
 * 
 * This file is part of Cshellsynth.
 * 
 * Cshellsynth is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 2 of the License, or
 * (at your option) any later version.
 * 
 * Cshellsynth is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 * 
 * You should have received a copy of the GNU General Public License
 * along with Cshellsynth.  If not, see <http://www.gnu.org/licenses/>.
 */
#ifndef CSHELLSYNTH_CONTROLLER_H
#define CSHELLSYNTH_CONTROLLER_H 1

#include <jack/jack.h>
#include <cshellsynth/atomic-types.h>
#include <cshellsynth/jclient.h>

/**
 * Controller
 *
 * Ruby version: @c Controllers::Controller
 *
 * See @ref jclient_t
 */
typedef struct cs_ctlr_struct {
    jack_client_t *client;
    jack_port_t *ctl_port; /* Output control port */
    jack_port_t *out_port; /* Output port */
} cs_ctlr_t;

/**
 * Destroy controller
 *
 * See @ref jclient_destroy
 */
#define cs_ctlr_destroy(cs_ctlr) jclient_destroy((jclient_t *) (cs_ctlr))

/**
 * Initialize controller
 *
 * See @ref jclient_init
 */
int cs_ctlr_init(cs_ctlr_t *self, const char *client_name, jack_options_t flags, char *server_name);

#endif
