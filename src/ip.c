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

#include "ip.h"

red_ip_t
*new_ip()
{
	red_ip_t *ip = malloc(sizeof(red_ip_t));
	init_ip(ip);
	return ip;
}

void
init_ip(red_ip_t *ip)
{
	ip->ip_address = NULL;
}

void
free_ip(red_ip_t *ip)
{
	sdsfree(ip->ip_address);
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
pack_ip(msgpack_packer *packer, red_ip_t *ip)
{
	msgpack_pack_map(packer, 1);
	pack_key_value(packer, "ip_address", 10, (char *)ip->ip_address, (int)sdslen(ip->ip_address));
}

static void
*list_dup_ip(void *ptr)
{
	red_ip_t *ip = (red_ip_t *)ptr;
	red_ip_t *dup_ip = malloc(sizeof(red_ip_t));
	dup_ip->ip_address = sdsdup(ip->ip_address);
	return (void *)dup_ip;
}

static void
list_free_ip(void *ptr)
{
	free_ip((red_ip_t *)ptr);
}

static int
list_match_ip(void *ptr, void *key)
{
	red_ip_t *ip = (red_ip_t *)ptr;
	if (sdscmp(ip->ip_address,(sds)key) == 0)
		return 1;
	else
		return 0;
}

red_ip_t
*unpack_ip(msgpack_object object)
{
	if (object.type != MSGPACK_OBJECT_MAP)
		return NULL;

	red_ip_t *ip = malloc(sizeof(red_ip_t));
	init_ip(ip);

	msgpack_object_kv* p    = object.via.map.ptr;
	msgpack_object_kv* pend = object.via.map.ptr + object.via.map.size;

	for (; p < pend; ++p) {
		if (p->key.type != MSGPACK_OBJECT_RAW || p->val.type != MSGPACK_OBJECT_RAW)
			continue;

		msgpack_object_raw *key = &(p->key.via.raw);
		msgpack_object_raw *val = &(p->val.via.raw);

		if (!strncmp(key->ptr, "ip_address", key->size)) {
			ip->ip_address = sdsnewlen(val->ptr, val->size);
		}
	}

	return ip;
}

list
*unpack_ips(msgpack_object object)
{
	list *ip_list = listCreate();
	listSetDupMethod(ip_list, list_dup_ip);
	listSetFreeMethod(ip_list, list_free_ip);
	listSetMatchMethod(ip_list, list_match_ip);
	if (object.type != MSGPACK_OBJECT_ARRAY)
		return ip_list;
	red_ip_t *ip;

	for (int i = 0; i < object.via.array.size; i++) {
		ip = unpack_ip(object.via.array.ptr[i]);
		if (ip) {
			listAddNodeTail(ip_list, (void *)ip);
		}
	}
	return ip_list;
}