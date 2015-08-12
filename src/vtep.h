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
#ifndef VTEP_H
#define VTEP_H

#include "util/sds.h"

// todo: move this to autobuild
#define VTEP_VERSION 	"0.0.2"

/* Error codes */
#define VTEP_OK	0
#define VTEP_ERR	-1

/* Sensible defaults */
#define VTEPD_DEFAULT_ADDR	"127.0.0.1"
#define VTEPD_DEFAULT_PORT	4470

typedef struct config_s {
	sds		vtepd_ip;	/* host/ip of vtep instance */
	int		vtepd_port;	/* port of vtep instance */
} config_t;

extern config_t config;
extern int exit_code;

#endif