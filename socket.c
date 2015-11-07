/*
 * Copyright (c) 2015 Mark Heily <mark@heily.com>
 *
 * Permission to use, copy, modify, and distribute this software for any
 * purpose with or without fee is hereby granted, provided that the above
 * copyright notice and this permission notice appear in all copies.
 *
 * THE SOFTWARE IS PROVIDED "AS IS" AND THE AUTHOR DISCLAIMS ALL WARRANTIES
 * WITH REGARD TO THIS SOFTWARE INCLUDING ALL IMPLIED WARRANTIES OF
 * MERCHANTABILITY AND FITNESS. IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR
 * ANY SPECIAL, DIRECT, INDIRECT, OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES
 * WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS, WHETHER IN AN
 * ACTION OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING OUT OF
 * OR IN CONNECTION WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.
 */

#include <sys/socket.h>
#include <sys/types.h>
#include <sys/event.h>

#include "job.h"
#include "socket.h"

/* The main kqueue descriptor used by launchd */
static int parent_kqfd;

/* The kqueue descriptor dedicated to socket activation */
static int socket_kqfd;

struct job_manifest_socket *
job_manifest_socket_new()
{
	struct job_manifest_socket *jms;

	jms = calloc(1, sizeof(*jms));
	if (!jms) return NULL;

	jms->sd = -1;
	jms->sock_type = SOCK_STREAM;
	jms->sock_passive = true;
	jms->sock_family = PF_INET;

	return (jms);
}

void job_manifest_socket_free(struct job_manifest_socket *jms)
{
	if (!jms) return;
	if (jms->sd >= 0) (void) close(jms->sd);
	free(jms->label);
	free(jms->sock_node_name);
	free(jms->sock_service_name);
	free(jms->sock_path_name);
	free(jms->secure_socket_with_key);
	free(jms->multicast_group);
	free(jms);
}

void setup_socket_activation(int kqfd)
{
	struct kevent kev;

	parent_kqfd = kqfd;
	socket_kqfd = kqueue();
	if (socket_kqfd < 0) abort();

	EV_SET(&kev, socket_kqfd, EVFILT_READ, EV_ADD, 0, 0, &setup_socket_activation);
	if (kevent(parent_kqfd, &kev, 1, NULL, 0, NULL) < 0) abort();
}
