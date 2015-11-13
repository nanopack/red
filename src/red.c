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

#include "red.h"
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
"Usage: red [OPTIONS] <subcommand>  <args> ...\n"
"    -h <hostname>      RED hostname (default: %s)\n"
"    -p <port>          RED port (default: %i)\n"
"    --help             Output this help and exit\n"
"    --version          Output version and exit\n"
"    --yaml             Format output in YAML\n"
"    --nooutput         Quiet the output for scripting\n"
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
	REDD_DEFAULT_ADDR,
	REDD_DEFAULT_PORT);

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
			sdsfree(config.redd_ip);
			config.redd_ip = sdsnew(argv[++i]);
		} else if (!strcmp(argv[i],"-h") && lastarg) {
			usage();
		} else if (!strcmp(argv[i],"--help")) {
			usage();
		} else if (!strcmp(argv[i],"-p") && !lastarg) {
			config.redd_port = atoi(argv[++i]);
		} else if (!strcmp(argv[i],"-v") || !strcmp(argv[i], "--version")) {
			printf("red %s\n", RED_VERSION);
			exit(0);
		} else if (!strcmp(argv[i],"-y") || !strcmp(argv[i], "--yaml")) {
			config.yaml_out = 1;
		} else if (!strcmp(argv[i],"-n") || !strcmp(argv[i], "--nooutput")) {
			config.no_output = 1;
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
	config.redd_ip   = strdup(REDD_DEFAULT_ADDR);
	config.redd_port = REDD_DEFAULT_PORT;
	config.yaml_out   = 0;
	config.no_output  = 0;
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
		handle_show_ip(argc, argv);

	else if (!strcmp(command, "show-node"))
		handle_show_node(argc, argv);

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
