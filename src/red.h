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
#ifndef RED_H
#define RED_H

#include "util/sds.h"

// todo: move this to autobuild
#define RED_VERSION 	"0.0.2"

/* Error codes */
#define RED_OK		0
#define RED_ERR	-1

/* Sensible defaults */
#define REDD_DEFAULT_ADDR	"127.0.0.1"
#define REDD_DEFAULT_PORT	4470

typedef struct config_s {
	sds		redd_ip;	/* host/ip of red instance */
	int		redd_port;	/* port of red instance */
	int		yaml_out;
	int		no_output;
} config_t;

extern config_t config;
extern int exit_code;

#endif