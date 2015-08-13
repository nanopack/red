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

#ifndef IP_H
#define IP_H

#include <msgpack.h>

#include "util/sds.h"
#include "util/adlist.h"

typedef struct vtep_ip_s {
	sds ip_address;
} vtep_ip_t;

vtep_ip_t	*new_ip();
void		init_ip(vtep_ip_t *ip);
void		free_ip(vtep_ip_t *ip);

void		pack_ip(vtep_ip_t *ip, int *len);
vtep_ip_t	*unpack_ip(msgpack_object object);
list		*unpack_ips(msgpack_object object);

#endif