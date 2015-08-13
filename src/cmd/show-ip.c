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
#include "cmd/show-ip.h"
#include "ip.h"

static list *ips;

static void 
usage(void)
{
	fprintf(stderr,
		"Usage:    show-ip\n");

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
unpack_data(char *data, int len)
{
	msgpack_zone mempool;
	msgpack_zone_init(&mempool, 4096);

	msgpack_object deserialized;
	msgpack_unpack(data, len, NULL, &mempool, &deserialized);

	if (deserialized.type == MSGPACK_OBJECT_MAP) {
		msgpack_object_kv* p = deserialized.via.map.ptr;
		msgpack_object_kv* const pend = deserialized.via.map.ptr + deserialized.via.map.size;

		for (; p < pend; ++p) {
			if (p->key.type == MSGPACK_OBJECT_RAW && p->val.type == MSGPACK_OBJECT_ARRAY) {
				if (!strncmp(p->key.via.raw.ptr, "ip_addresses", p->key.via.raw.size)) {
					ips = unpack_ips(p->val);
				}
			}
		}
	}
}

static void
print_data()
{
	vtep_ip_t *ip;
	listNode *list_node;
	listIter *itr = listGetIterator(ips, AL_START_HEAD);
	printf("IP ADDRESSES:\n");
	while ((list_node = listNext(itr)) != NULL) {
		ip = (vtep_ip_t *)list_node->value;
		printf("%s\n", ip->ip_address);
	}
	listReleaseIterator(itr);
	listRelease(ips);
}

static void
on_response(msgxchng_response_t *res, int status)
{
	if (status == VTEP_ERR)
		exit(1);
	
	unpack_data(res->data, res->data_len);
	print_data();

	clean_msgxchng_response(res);
	free(res);
}

void 
handle_show_ip(int argc, char **argv)
{
	parse_options(argc, argv);

	msgxchng_request_t *req;
	req = new_msgxchng_request("1", 1, "ip.list", 7, "", 0);

	vtepd_request(req, on_response);
}