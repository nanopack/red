// -*- mode: c; tab-width: 4; indent-tabs-mode: 1; st-rulers: [70] -*-
// vim: ts=8 sw=8 ft=c noet
/*
 * CDDL HEADER START
 *
 * The contents of this file are subject to the terms of the
 * Common Development and Distribution License (the "License").
 * You may not use this file except in compliance with the License.
 *
 * You can obtain a copy of the license at usr/src/OPENSOLARIS.LICENSE
 * or http://www.opensolaris.org/os/licensing.
 * See the License for the specific language governing permissions
 * and limitations under the License.
 *
 * When distributing Covered Code, include this CDDL HEADER in each
 * file and include the License file at usr/src/OPENSOLARIS.LICENSE.
 * If applicable, add the following below this CDDL HEADER, with the
 * fields enclosed by brackets "[]" replaced with your own identifying
 * information: Portions Copyright [yyyy] [name of copyright owner]
 *
 * CDDL HEADER END
 */
/*
 * Copyright 2013 Pagoda Box, Inc.  All rights reserved.
 */

#ifndef NODE_H
#define NODE_H

#include <msgpack.h>

#include "util/sds.h"
#include "util/adlist.h"

typedef struct vtep_node_s {
	sds hostname;
} vtep_node_t;

vtep_node_t	*new_node();
void		init_node(vtep_node_t *node);
void		free_node(vtep_node_t *node);

char		*pack_node(msgpack_packer *packer, vtep_node_t *node);
vtep_node_t	*unpack_node(msgpack_object object);
list		*unpack_nodes(msgpack_object object);

#endif