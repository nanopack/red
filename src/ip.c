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

#include "ip.h"

vtep_ip_t
*new_ip()
{
	vtep_ip_t *ip = malloc(sizeof(vtep_ip_t));
	init_ip(ip);
	return ip;
}

void
init_ip(vtep_ip_t *ip)
{
	ip->ip_address = NULL;
}

void
free_ip(vtep_ip_t *ip)
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
pack_ip(msgpack_packer *packer, vtep_ip_t *ip)
{
	msgpack_pack_map(packer, 1);
	pack_key_value(packer, "ip_address", 10, (char *)ip->ip_address, (int)sdslen(ip->ip_address));
}

static void
*list_dup_ip(void *ptr)
{
	vtep_ip_t *ip = (vtep_ip_t *)ptr;
	vtep_ip_t *dup_ip = malloc(sizeof(vtep_ip_t));
	dup_ip->ip_address = sdsdup(ip->ip_address);
	return (void *)dup_ip;
}

static void
list_free_ip(void *ptr)
{
	free_ip((vtep_ip_t *)ptr);
}

static int
list_match_ip(void *ptr, void *key)
{
	vtep_ip_t *ip = (vtep_ip_t *)ptr;
	if (sdscmp(ip->ip_address,(sds)key) == 0)
		return 1;
	else
		return 0;
}

vtep_ip_t
*unpack_ip(msgpack_object object)
{
	if (object.type != MSGPACK_OBJECT_MAP)
		return NULL;

	vtep_ip_t *ip = malloc(sizeof(vtep_ip_t));
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
	vtep_ip_t *ip;

	for (int i = 0; i < object->val.via.array.size; i++) {
		ip = unpack_ip(object->val.via.array.ptr[i]);
		if (ip) {
			listAddNodeTail(ip_list, (void *)ip);
		}
	}
	return ip_list;
}