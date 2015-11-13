// -*- mode: c; tab-width: 4; indent-tabs-mode: 1; st-rulers: [70] -*-
// vim: ts=8 sw=8 ft=c noet
/*
 * Copyright (c) 2015 Pagoda Box Inc
 * 
 * This Source Code Form is subject to the terms of the Mozilla Public License, v.
 * 2.0. If a copy of the MPL was not distributed with this file, You can obtain one
 * at http://mozilla.org/MPL/2.0/.
 */

#include <stdlib.h>
#include <uv.h>
#include <bframe.h>
#include <msgxchng.h>

#include "redd.h"
#include "red.h"

typedef struct redd_session_s {
	msgxchng_request_t 	*req;		/* msgxchng request */
	redd_callback		cb;			/* callback function to handle response */
	uv_tcp_t 			*socket;	/* tcp socket */
	uv_stream_t			*stream;	/* connection stream */
	bframe_buffer_t		*buf;		/* buffer between packets */
	void				*data;		/* data storage */
} redd_session_t;

static redd_session_t *
new_redd_session(msgxchng_request_t *req, redd_callback cb)
{
	redd_session_t *session = (redd_session_t *)malloc(sizeof(redd_session_t));

	session->req    = req;
	session->cb     = cb;
	session->socket = NULL;
	session->stream = NULL;
	session->buf    = new_bframe_buffer();
	session->data   = NULL;

	return session;
}

static void
clean_redd_session(redd_session_t *session)
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
parse_response(redd_session_t *session, bframe_t *frame)
{
	msgxchng_response_t *res;

	res = unpack_msgxchng_response(frame->data, frame->len.int_len);

	if (!strcmp(res->status, "complete")) {
		uv_close((uv_handle_t *)session->stream, (uv_close_cb) free);
		uv_stop(uv_default_loop());
		clean_redd_session(session);
	}

	session->cb(res, RED_OK);

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
	redd_session_t *session = (redd_session_t *)proto->data;

	if (nread < 0) {
		if (buf.base) {
      		free(buf.base);
      		buf.base = NULL;
    	}
    	printf("Error: Connection closed prematurely\n");
		uv_close((uv_handle_t *)proto, (uv_close_cb) free);
		session->cb(NULL, RED_ERR);
		uv_stop(uv_default_loop());
		clean_redd_session(session);
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
redd_recv_response(redd_session_t *session)
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
redd_send_request(redd_session_t *session)
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
	redd_session_t *session = (redd_session_t *)connection->data;

	if (status < 0) {
		printf("Error: Unable to connect to RED\n");
		session->cb(NULL, RED_ERR);
		uv_stop(uv_default_loop());
		clean_redd_session(session);
		free(session->socket);
	} else {
		session->stream   = (uv_stream_t *)connection->handle;

		redd_send_request(session);
		redd_recv_response(session);
	}

	free(connection);
}

static void
redd_connect(char *ip, redd_session_t *session)
{
	uv_tcp_t *socket = (uv_tcp_t *)malloc(sizeof(uv_tcp_t));

	uv_tcp_init(uv_default_loop(), socket);
	uv_tcp_keepalive(socket, 1, 60);

	struct sockaddr_in dest = uv_ip4_addr(ip, config.redd_port);

	uv_connect_t *connect = malloc(sizeof(uv_connect_t));
	if(uv_tcp_connect(connect, socket, dest, on_connect) == UV_OK) {
		session->socket = socket;
		connect->data = (void *)session;
	}
}

static void
on_resolve(uv_getaddrinfo_t *resolver, int status, struct addrinfo *res)
{
	redd_session_t *session = (redd_session_t *)resolver->data;

	if (status < 0) {
		printf("Error: Unable to resolve %s\n", config.redd_ip);
		session->cb(NULL, RED_ERR);
		uv_stop(uv_default_loop());
		clean_redd_session(session);
	} else {
		char addr[17] = {'\0'};
		uv_ip4_name((struct sockaddr_in*) res->ai_addr, addr, 16);

		redd_connect(addr, session);
	}

	/* cleanup */
	free(session->data);
	free(resolver);
	uv_freeaddrinfo(res);
}

static void
redd_resolve(redd_session_t *session)
{
	uv_getaddrinfo_t *resolver = (uv_getaddrinfo_t *)malloc(sizeof(uv_getaddrinfo_t));
	struct addrinfo *hints     = (struct addrinfo *)malloc(sizeof(struct addrinfo));

	hints->ai_family   = PF_INET;
	hints->ai_socktype = SOCK_STREAM;
	hints->ai_protocol = IPPROTO_TCP;
	hints->ai_flags    = 0;

	if (uv_getaddrinfo(uv_default_loop(), resolver, on_resolve, config.redd_ip, NULL, hints) == UV_OK) {
		session->data = hints;
		resolver->data = session;
	}
}

static void
redd_run(redd_session_t *session)
{
	redd_resolve(session);

	uv_run(uv_default_loop(), UV_RUN_DEFAULT);
}

void
redd_request(msgxchng_request_t *req, redd_callback cb)
{
	redd_session_t *session = new_redd_session(req, cb);

	redd_run(session);
}
