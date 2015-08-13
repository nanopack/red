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

#include <stdlib.h>
#include <uv.h>
#include <bframe.h>
#include <msgxchng.h>

#include "vtepd.h"
#include "vtep.h"

typedef struct vtepd_session_s {
	msgxchng_request_t 	*req;		/* msgxchng request */
	vtepd_callback		cb;			/* callback function to handle response */
	uv_tcp_t 			*socket;	/* tcp socket */
	uv_stream_t			*stream;	/* connection stream */
	bframe_buffer_t		*buf;		/* buffer between packets */
	void				*data;		/* data storage */
} vtepd_session_t;

static vtepd_session_t *
new_vtepd_session(msgxchng_request_t *req, vtepd_callback cb)
{
	vtepd_session_t *session = (vtepd_session_t *)malloc(sizeof(vtepd_session_t));

	session->req    = req;
	session->cb     = cb;
	session->socket = NULL;
	session->stream = NULL;
	session->buf    = new_bframe_buffer();
	session->data   = NULL;

	return session;
}

static void
clean_vtepd_session(vtepd_session_t *session)
{
	clean_msgxchng_request(session->req);
	free(session->req);
	session->req = NULL;

	clean_bframe_buffer(session->buf);
	free(session->buf);
	session->buf = NULL;

	free(session);
	session = NULL;
}

static void
parse_response(vtepd_session_t *session, bframe_t *frame)
{
	msgxchng_response_t *res;

	res = unpack_msgxchng_response(frame->data, frame->len.int_len);

	if (!strcmp(res->status, "complete")) {
		uv_close((uv_handle_t *)session->stream, (uv_close_cb) free);
		uv_stop(uv_default_loop());
		clean_vtepd_session(session);
	}

	session->cb(res, VTEP_OK);

	/* cleanup */
	clean_bframe(frame);
	free(frame);
	frame = NULL;
}

static uv_buf_t
read_alloc_buffer(uv_handle_t *handle, size_t suggested_size)
{
	return uv_buf_init((char *)malloc(suggested_size), suggested_size);
}

static void
on_read(uv_stream_t *proto, ssize_t nread, uv_buf_t buf)
{
	vtepd_session_t *session = (vtepd_session_t *)proto->data;

	if (nread < 0) {
		if (buf.base) {
      		free(buf.base);
      		buf.base = NULL;
    	}
    	printf("Error: Connection closed prematurely\n");
		uv_close((uv_handle_t *)proto, (uv_close_cb) free);
		session->cb(NULL, VTEP_ERR);
		uv_stop(uv_default_loop());
		clean_vtepd_session(session);
		return;
	}

	if (nread == 0) {
		free(buf.base);
		buf.base = NULL;
		return;
	}

	int framec;
	bframe_t **frames;

	frames = parse_char_to_bframes(buf.base, nread, session->buf, &framec);

	int i;
	for (i=0; i < framec ; i++) 
		parse_response(session, frames[i]);

	/* cleanup */
	free(buf.base);
	buf.base = NULL;

	free(frames);
	frames = NULL;
}

static void
vtepd_recv_response(vtepd_session_t *session)
{
	if (uv_read_start(session->stream, read_alloc_buffer, on_read) == UV_OK)
		session->stream->data = session;
}

static void
on_write(uv_write_t* req, int status)
{
	uv_buf_t *buf = (uv_buf_t *)req->data;
	free(buf->base);
	buf->base = NULL;
	free(req->data);
	req->data = NULL;
	free(req);
	req = NULL;
}

static void
vtepd_send_request(vtepd_session_t *session)
{
	int p_len;
	char *payload = pack_msgxchng_request(session->req, &p_len);

	bframe_t *frame;
	frame = new_bframe(payload, p_len);

	uv_buf_t *buf;
	uv_write_t *writer;

	buf    = (uv_buf_t *)malloc(sizeof(uv_buf_t));
	writer = (uv_write_t *)malloc(sizeof(uv_write_t));

	int size;
	buf->base = bframe_to_char(frame, &size);
	buf->len = size;

	if (uv_write(writer, session->stream, buf, 1, on_write) == UV_OK)
		writer->data = (void *)buf;

	clean_bframe(frame);
	free(frame);
	frame = NULL;
}

static void 
on_connect(uv_connect_t* connection, int status)
{
	vtepd_session_t *session = (vtepd_session_t *)connection->data;

	if (status < 0) {
		printf("Error: Unable to connect to VTEP\n");
		session->cb(NULL, VTEP_ERR);
		uv_stop(uv_default_loop());
		clean_vtepd_session(session);
		free(session->socket);
	} else {
		session->stream   = (uv_stream_t *)connection->handle;

		vtepd_send_request(session);
		vtepd_recv_response(session);
	}

	free(connection);
}

static void
vtepd_connect(char *ip, vtepd_session_t *session)
{
	uv_tcp_t *socket = (uv_tcp_t *)malloc(sizeof(uv_tcp_t));

	uv_tcp_init(uv_default_loop(), socket);
	uv_tcp_keepalive(socket, 1, 60);

	struct sockaddr_in dest = uv_ip4_addr(ip, config.vtepd_port);

	uv_connect_t *connect = malloc(sizeof(uv_connect_t));
	if(uv_tcp_connect(connect, socket, dest, on_connect) == UV_OK) {
		session->socket = socket;
		connect->data = (void *)session;
	}
}

static void
on_resolve(uv_getaddrinfo_t *resolver, int status, struct addrinfo *res)
{
	vtepd_session_t *session = (vtepd_session_t *)resolver->data;

	if (status < 0) {
		printf("Error: Unable to resolve %s\n", config.vtepd_ip);
		session->cb(NULL, VTEP_ERR);
		uv_stop(uv_default_loop());
		clean_vtepd_session(session);
	} else {
		char addr[17] = {'\0'};
		uv_ip4_name((struct sockaddr_in*) res->ai_addr, addr, 16);

		vtepd_connect(addr, session);
	}

	/* cleanup */
	free(session->data);
	free(resolver);
	uv_freeaddrinfo(res);
}

static void
vtepd_resolve(vtepd_session_t *session)
{
	uv_getaddrinfo_t *resolver = (uv_getaddrinfo_t *)malloc(sizeof(uv_getaddrinfo_t));
	struct addrinfo *hints     = (struct addrinfo *)malloc(sizeof(struct addrinfo));

	hints->ai_family   = PF_INET;
	hints->ai_socktype = SOCK_STREAM;
	hints->ai_protocol = IPPROTO_TCP;
	hints->ai_flags    = 0;

	if (uv_getaddrinfo(uv_default_loop(), resolver, on_resolve, config.vtepd_ip, NULL, hints) == UV_OK) {
		session->data = hints;
		resolver->data = session;
	}
}

static void
vtepd_run(vtepd_session_t *session)
{
	vtepd_resolve(session);

	uv_run(uv_default_loop(), UV_RUN_DEFAULT);
}

void
vtepd_request(msgxchng_request_t *req, vtepd_callback cb)
{
	vtepd_session_t *session = new_vtepd_session(req, cb);

	vtepd_run(session);
}
