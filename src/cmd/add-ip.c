// -*- mode: c; tab-width: 4; indent-tabs-mode: 1; st-rulers: [70] -*-
// vim: ts=8 sw=8 ft=c noet
/*
 * Copyright (c) 2015 Pagoda Box Inc
 * 
 * This Source Code Form is subject to the terms of the Mozilla Public License, v.
 * 2.0. If a copy of the MPL was not distributed with this file, You can obtain one
 * at http://mozilla.org/MPL/2.0/.
 */

#include <stdio.h>	/* standard buffered input/output */
#include <stdlib.h>	/* standard library definitions */
#include <string.h>	/* string operations */
#include <msgxchng.h>

#include "util/sds.h"
#include "redd.h"
#include "red.h"
#include "cmd/add-ip.h"
#include "ip.h"

static red_ip_t ip;

static void 
usage(void)
{
	fprintf(stderr,
		"Usage:    add-ip <ip address>\n");

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
	if (status == RED_ERR)
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
					if (!config.no_output) fprintf(stderr, "red: %s\n", p->val.via.raw.ptr);
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
handle_add_ip(int argc, char **argv)
{
	char *data;
	int size;
	init_ip(&ip);
	parse_options(argc, argv);
	if (ip.ip_address == NULL)
		usage();

	msgxchng_request_t *req;
	data = pack_data(&size);
	req = new_msgxchng_request("1", 1, "ip.add", 6, data, size);

	free(data);
	redd_request(req, on_response);
}