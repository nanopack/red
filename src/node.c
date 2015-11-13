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
#include "node.h"

red_node_t
*new_node()
{
	red_node_t *node = malloc(sizeof(red_node_t));
	init_node(node);
	return node;
}

void
init_node(red_node_t *node)
{
	node->hostname = NULL;
}

void
free_node(red_node_t *node)
{
	sdsfree(node->hostname);
}

static void
pack_key_value(msgpack_packer *packer, char *key, 
	int key_len, char *value, int value_len)
{
	msgpack_pack_raw(packer, key_len);
	msgpack_pack_raw_body(packer, key, key_len);
	msgpack_pack_raw(packer, value_len);
	msgpack_pack_raw_body(packer, value, value_len);
}

void
pack_node(msgpack_packer *packer, red_node_t *node)
{
	msgpack_pack_map(packer, 1);
	pack_key_value(packer, "node", 4, (char *)node->hostname, (int)sdslen(node->hostname));
}

static void
*list_dup_node(void *ptr)
{
	red_node_t *node = (red_node_t *)ptr;
	red_node_t *dup_node = malloc(sizeof(red_node_t));
	dup_node->hostname = sdsdup(node->hostname);
	return (void *)dup_node;
}

static void
list_free_node(void *ptr)
{
	free_node((red_node_t *)ptr);
}

static int
list_match_node(void *ptr, void *key)
{
	red_node_t *node = (red_node_t *)ptr;
	if (sdscmp(node->hostname,(sds)key) == 0)
		return 1;
	else
		return 0;
}

red_node_t
*unpack_node(msgpack_object object)
{
	if (object.type != MSGPACK_OBJECT_MAP)
		return NULL;

	red_node_t *node = malloc(sizeof(red_node_t));
	init_node(node);

	msgpack_object_kv* p    = object.via.map.ptr;
	msgpack_object_kv* pend = object.via.map.ptr + object.via.map.size;

	for (; p < pend; ++p) {
		if (p->key.type != MSGPACK_OBJECT_RAW || p->val.type != MSGPACK_OBJECT_RAW)
			continue;

		msgpack_object_raw *key = &(p->key.via.raw);
		msgpack_object_raw *val = &(p->val.via.raw);

		if (!strncmp(key->ptr, "node", key->size)) {
			node->hostname = sdsnewlen(val->ptr, val->size);
		}
	}

	return node;
}

list
*unpack_nodes(msgpack_object object)
{
	list *node_list = listCreate();
	listSetDupMethod(node_list, list_dup_node);
	listSetFreeMethod(node_list, list_free_node);
	listSetMatchMethod(node_list, list_match_node);
	if (object.type != MSGPACK_OBJECT_ARRAY)
		return node_list;
	red_node_t *node;

	for (int i = 0; i < object.via.array.size; i++) {
		node = unpack_node(object.via.array.ptr[i]);
		if (node) {
			listAddNodeTail(node_list, (void *)node);
		}
	}
	return node_list;
}