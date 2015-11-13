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
#include "cmd/show-node.h"
#include "node.h"

static list *nodes;

static void 
usage(void)
{
	fprintf(stderr,
		"Usage:    show-node\n");

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
				if (!strncmp(p->key.via.raw.ptr, "nodes", p->key.via.raw.size)) {
					nodes = unpack_nodes(p->val);
				}
			}
		}
	}
}

static void
print_data()
{
	red_node_t *node;
	listNode *list_node;
	listIter *itr = listGetIterator(nodes, AL_START_HEAD);
	printf("NODES:\n");
	while ((list_node = listNext(itr)) != NULL) {
		node = (red_node_t *)list_node->value;
		printf("%s\n", node->hostname);
	}
	listReleaseIterator(itr);
	listRelease(nodes);
}

static void
print_yaml_data()
{
	red_node_t *node;
	listNode *list_node;
	listIter *itr = listGetIterator(nodes, AL_START_HEAD);
	printf("nodes:\n");
	while ((list_node = listNext(itr)) != NULL) {
		node = (red_node_t *)list_node->value;
		printf("  - %s\n", node->hostname);
	}
	listReleaseIterator(itr);
	listRelease(nodes);
}

static void
on_response(msgxchng_response_t *res, int status)
{
	if (status == RED_ERR)
		exit(1);

	unpack_data(res->data, res->data_len);
	if (config.yaml_out) {
		print_yaml_data();
	} else {
		print_data();
	}

	clean_msgxchng_response(res);
	free(res);
}

void 
handle_show_node(int argc, char **argv)
{
	parse_options(argc, argv);

	msgxchng_request_t *req;
	req = new_msgxchng_request("1", 1, "node.list", 9, "", 0);

	redd_request(req, on_response);
}