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
#include "cmd/remove-node.h"
#include "node.h"

static vtep_node_t node;

static void 
usage(void)
{
	fprintf(stderr,
		"Usage:    remove-node <hostname | ip address>\n");

	exit(1);
}

static void 
parse_options(int argc, char **argv) 
{
	int i;
	node.hostname = NULL;

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
				node.hostname = sdsnew(argv[i]);
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

	pack_node(packer, &node);

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

	msgpack_zone mempool;
	msgpack_zone_init(&mempool, 4096);

	msgpack_object deserialized;
	msgpack_unpack(res->data, res->data_len, NULL, &mempool, &deserialized);

	int success = 0;

	if (deserialized.type == MSGPACK_OBJECT_MAP) {
		msgpack_object_kv* p = deserialized.via.map.ptr;
		msgpack_object_kv* const pend = deserialized.via.map.ptr + deserialized.via.map.size;

		for (; p < pend; ++p) {
			if (p->key.type == MSGPACK_OBJECT_RAW && p->val.type == MSGPACK_OBJECT_RAW) {
				if (!strncmp(p->key.via.raw.ptr, "return", p->key.via.raw.size)) {
					if (!strncmp(p->val.via.raw.ptr, "success", p->val.via.raw.size))
						success = 1;
					else 
						success = 0;
				} else if (!strncmp(p->key.via.raw.ptr, "error", p->key.via.raw.size)) {
					if (!config.no_output) fprintf(stderr, "vtep: %s\n", p->val.via.raw.ptr);
				}
			}
		}
	}

	clean_msgxchng_response(res);
	free(res);

	if (!success)
		exit(1);
}

void 
handle_remove_node(int argc, char **argv)
{
	char *data;
	int size;
	init_node(&node);
	parse_options(argc, argv);
	if (node.hostname == NULL)
		usage();

	msgxchng_request_t *req;
	data = pack_data(&size);
	req = new_msgxchng_request("1", 1, "node.remove", 11, data, size);

	free(data);
	vtepd_request(req, on_response);
}