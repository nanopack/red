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

#include <stdio.h>	/* standard buffered input/output */
#include <stdlib.h>	/* standard library definitions */
#include <string.h>	/* string operations */
#include <msgxchng.h>

#include "util/sds.h"
#include "vtepd.h"
#include "vtep.h"
#include "cmd/ping.h"

static void 
usage(void)
{
	fprintf(stderr,
		"Usage:    ping\n");

	exit(1);
}

static void 
parse_options(int argc, char **argv) 
{
	int i;

	for (i = 0; i < argc; i++) {
		int lastarg = i==argc-1;

		if (!strcmp(argv[i], "-h") && lastarg) {
			usage();
		} else if (!strcmp(argv[i], "--help")) {
			usage();
		} else {
			fprintf(stderr,
				"Unrecognized option: '%s'\n",
				argv[i]);
			usage();
		}
	}
}

static void
on_response(msgxchng_response_t *res, int status)
{
	if (status == VTEP_ERR)
		exit(1);

	if (!config.no_output) printf("pong!\n");

	clean_msgxchng_response(res);
	free(res);
}

void 
handle_ping(int argc, char **argv)
{
	parse_options(argc, argv);

	msgxchng_request_t *req;
	req = new_msgxchng_request("1", 1, "ping", 4, "", 0);

	vtepd_request(req, on_response);
}
