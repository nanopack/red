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

#include "ip.h"
#include "node.h"
#include "util/sds.h"
#include "vtepd.h"
#include "vtep.h"
#include "cmd/status.h"
#include "util/adlist.h"

typedef struct vtep_status_s {
	list *ips;
	list *nodes;
	char *tun_dev;
	char *vxlan_dev;
	char *vxlan_vni;
	char *vxlan_group;
	char *vxlan_port;
	char *vxlan_interface;
} vtep_status_t;

static vtep_status_t vtep_status;

static void 
usage(void)
{
	fprintf(stderr,
		"Usage:    status\n");

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
initialize_data()
{
	vtep_status.ips = listCreate();
	vtep_status.nodes = listCreate();
	vtep_status.tun_dev = strdup("");
	vtep_status.vxlan_dev = strdup("");
	vtep_status.vxlan_vni = strdup("");
	vtep_status.vxlan_group = strdup("");
	vtep_status.vxlan_port = strdup("");
	vtep_status.vxlan_interface = strdup("");
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
					listRelease(vtep_status.ips);
					vtep_status.ips = unpack_ips(p->val);
				} else if (!strncmp(p->key.via.raw.ptr, "nodes", p->key.via.raw.size)) {
					listRelease(vtep_status.nodes);
					vtep_status.nodes = unpack_nodes(p->val);
				}
			} else if (p->key.type == MSGPACK_OBJECT_RAW && p->val.type == MSGPACK_OBJECT_RAW) {
				 else if (!strncmp(p->key.via.raw.ptr, "tun_dev", p->key.via.raw.size)) {
					free(vtep_status.tun_dev);
					vtep_status.tun_dev = strndup(p->val.via.raw.ptr, p->val.via.raw.size);
				} else if (!strncmp(p->key.via.raw.ptr, "vxlan_dev", p->key.via.raw.size)) {
					free(vtep_status.vxlan_dev);
					vtep_status.vxlan_dev = strndup(p->val.via.raw.ptr, p->val.via.raw.size);
				} else if (!strncmp(p->key.via.raw.ptr, "vxlan_vni", p->key.via.raw.size)) {
					free(vtep_status.vxlan_vni);
					vtep_status.vxlan_vni = strndup(p->val.via.raw.ptr, p->val.via.raw.size);
				} else if (!strncmp(p->key.via.raw.ptr, "vxlan_group", p->key.via.raw.size)) {
					free(vtep_status.vxlan_group);
					vtep_status.vxlan_group = strndup(p->val.via.raw.ptr, p->val.via.raw.size);
				} else if (!strncmp(p->key.via.raw.ptr, "vxlan_port", p->key.via.raw.size)) {
					free(vtep_status.vxlan_port);
					vtep_status.vxlan_port = strndup(p->val.via.raw.ptr, p->val.via.raw.size);
				} else if (!strncmp(p->key.via.raw.ptr, "vxlan_interface", p->key.via.raw.size)) {
					free(vtep_status.vxlan_interface);
					vtep_status.vxlan_interface = strndup(p->val.via.raw.ptr, p->val.via.raw.size);
				}
			}
		}
	}
}

static void print_status()
{
	listIter *iterator	= listGetIterator(vtep_status.ips, AL_START_HEAD);
	listNode *list_node	= NULL;
	printf("Tunnel Device:\t%s\n", vtep_status.tun_dev);
	printf("VxLAN Device:\t%s\n", vtep_status.vxlan_dev);
	printf("VxLAN VNI:\t%s\n", vtep_status.vxlan_vni);
	printf("Multicast Group:\t%s\n", vtep_status.vxlan_group);
	printf("VxLan Port:\t%s\n", vtep_status.vxlan_port);
	printf("Real Interface:\t%s\n", vtep_status.vxlan_interface);
	iterator = listGetIterator(vtep_status.ips, AL_START_HEAD);
	printf("IP ADDRESSES:\n");
	while ((list_node = listNext(iterator)) != NULL) {
		vtep_ip_t *ip = (vtep_ip_t *)list_node->value;
		printf("\t%s\n", ip->ip_address);
	}
	listReleaseIterator(iterator);
	iterator = listGetIterator(vtep_status.nodes, AL_START_HEAD);
	printf("NODES:\n");
	while ((list_node = listNext(iterator)) != NULL) {
		vtep_node_t *node = (vtep_node_t *)list_node->value;
		printf("\t%s\n", node->hostname);
	}
	listReleaseIterator(iterator);
	listRelease(vtep_status.ips);
	listRelease(vtep_status.nodes);
	free(vtep_status.tun_dev);
	free(vtep_status.vxlan_dev);
	free(vtep_status.vxlan_vni);
	free(vtep_status.vxlan_group);
	free(vtep_status.vxlan_port);
	free(vtep_status.vxlan_interface);
}

static void
on_response(msgxchng_response_t *res, int status)
{
	if (status == VTEP_ERR)
		exit(1);

	initialize_data();
	unpack_data(res->data, res->data_len);
	print_status();

	clean_msgxchng_response(res);
	free(res);
}

void 
handle_status(int argc, char **argv)
{
	parse_options(argc, argv);

	msgxchng_request_t *req;
	req = new_msgxchng_request("1", 1, "status", 6, "", 0);

	vtepd_request(req, on_response);
}
