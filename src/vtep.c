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

#include "vtep.h"
#include "util/sds.h"
#include "cmd/add-ip.h"
#include "cmd/add-node.h"
#include "cmd/remove-ip.h"
#include "cmd/remove-node.h"
#include "cmd/show-ip.h"
#include "cmd/show-node.h"
#include "cmd/ping.h"
#include "cmd/status.h"

config_t config;
int exit_code;

static void 
usage(void)
{
	fprintf(stderr,
"Usage: vtep [OPTIONS] <subcommand>  <args> ...\n"
"    -h <hostname>      VTEP hostname (default: %s)\n"
"    -p <port>          VTEP port (default: %i)\n"
"    --help             Output this help and exit\n"
"    --version          Output version and exit\n"
"\n"
"    add-ip <ip address>\n"
"    remove-ip <ip address>\n"
"    show-ip\n"
"\n"
"    add-node <ip address | hostname>\n"
"    remove-node <ip address | hostname>\n"
"    show-node\n"
"\n"
"    ping\n"
"\n"
"    status\n",
	VTEPD_DEFAULT_ADDR, 
	VTEPD_DEFAULT_PORT);

	exit(1);
}

static int 
parse_options(int argc, char **argv) 
{
	int i;

	if (argc == 1)
		usage();

	for (i = 1; i < argc; i++) {
		int lastarg = i==argc-1;

		if (!strcmp(argv[i],"-h") && !lastarg) {
			sdsfree(config.vtepd_ip);
			config.vtepd_ip = sdsnew(argv[++i]);
		} else if (!strcmp(argv[i],"-h") && lastarg) {
			usage();
		} else if (!strcmp(argv[i],"--help")) {
			usage();
		} else if (!strcmp(argv[i],"-p") && !lastarg) {
			config.vtepd_port = atoi(argv[++i]);
		} else if (!strcmp(argv[i],"-v") || !strcmp(argv[i], "--version")) {
			printf("vtep %s\n", VTEP_VERSION);
			exit(0);
		} else {
			if (argv[i][0] == '-') {
				fprintf(stderr,
					"Unrecognized option or bad number of args for: '%s'\n",
					argv[i]);
				exit(1);
			} else {
				/* Likely the command name, stop here. */
				break;
			}
		}
	}

	return i;
}

static void 
init_config(void)
{
	config.vtepd_ip   = strdup(VTEPD_DEFAULT_ADDR);
	config.vtepd_port = VTEPD_DEFAULT_PORT;
}

static void
handle_command(int argc, char **argv)
{
	char *command = argv[0];

	argc -= 1;
	argv += 1;

	if (!strcmp(command, "ping"))
		handle_ping(argc, argv);

	else if (!strcmp(command, "add-ip"))
		handle_add_ip(argc, argv);

	else if (!strcmp(command, "add-node"))
		handle_add_node(argc, argv);

	else if (!strcmp(command, "remove-ip"))
		handle_remove_ip(argc, argv);

	else if (!strcmp(command, "remove-node"))
		handle_remove_node(argc, argv);

	else if (!strcmp(command, "show-ip"))
		handle_remove_ip(argc, argv);

	else if (!strcmp(command, "show-node"))
		handle_remove_node(argc, argv);

	else if (!strcmp(command, "status"))
		handle_status(argc, argv);

	else
		usage();
}

int 
main(int argc, char **argv)
{
	exit_code = 0;
	int firstarg;

	init_config();

	firstarg = parse_options(argc, argv);
	argc     -= firstarg;
	argv     += firstarg;

	handle_command(argc, argv);

	return exit_code;
}

