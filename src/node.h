// -*- mode: c; tab-width: 4; indent-tabs-mode: 1; st-rulers: [70] -*-
// vim: ts=8 sw=8 ft=c noet
/*
 * Copyright (c) 2015 Pagoda Box Inc
 * 
 * This Source Code Form is subject to the terms of the Mozilla Public License, v.
 * 2.0. If a copy of the MPL was not distributed with this file, You can obtain one
 * at http://mozilla.org/MPL/2.0/.
 */

#ifndef NODE_H
#define NODE_H

#include <msgpack.h>

#include "util/sds.h"
#include "util/adlist.h"

typedef struct red_node_s {
	sds hostname;
} red_node_t;

red_node_t	*new_node();
void		init_node(red_node_t *node);
void		free_node(red_node_t *node);

void		pack_node(msgpack_packer *packer, red_node_t *node);
red_node_t	*unpack_node(msgpack_object object);
list		*unpack_nodes(msgpack_object object);

#endif