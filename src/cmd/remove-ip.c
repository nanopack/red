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
#include "cmd/remove-ip.h"
#include "ip.h"

static vtep_ip_t ip;

static void 
usage(void)
{
	fprintf(stderr,
		"Usage:    remove-ip <ip address>\n");

	exit(1);
}

static void 
parse_options(int argc, char **argv) 
{
	int i;
	ip.ip_address = NULL;

	for (i = 0; i < argc; i++) {
		int lastarg = i==argc-1;

		if (!strcmp(argv[i], "-h") && lastarg) {
			usage();
		} else if (!strcmp(argv[i], "--help")) {
			usage();
		} else {
			if (argv[i][0] == '-') {
				fprintf(stderr,
					"Unrecognized option: '%s'\n",
					argv[i]);
				usage();
			} else {
				ip.ip_address = sdsnew(argv[i]);
			}
		}
	}
}

static char *
pack_data(int *size)
{
	msgpack_sbuffer *buffer = NULL;
	msgpack_packer *packer  = NULL;
	char *data;

	buffer = msgpack_sbuffer_new();
	msgpack_sbuffer_init(buffer);
	packer = msgpack_packer_new((void *)buffer, msgpack_sbuffer_write);

	pack_ip(packer, &ip);

	data = (char *)malloc(buffer->size + 1);
	memcpy(data, &buffer->data[0], buffer->size);
	data[buffer->size] = '\0';
	*size = buffer->size;

	msgpack_packer_free(packer);
	msgpack_sbuffer_free(buffer);
	return data;
}

static void
on_response(msgxchng_response_t *res, int status)
{
	if (status == VTEP_ERR)
		exit(1);

	printf("status: %s\n", res->data);

	clean_msgxchng_response(res);
	free(res);
}

void 
handle_remove_ip(int argc, char **argv)
{
	char *data;
	int size;
	init_ip(&ip);
	parse_options(argc, argv);
	if (ip.ip_address == NULL)
		usage();

	msgxchng_request_t *req;
	data = pack_data(&size);
	req = new_msgxchng_request("1", 1, "ip.remove", 9, data, size);

	free(data);
	vtepd_request(req, on_response);
}