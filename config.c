/*
parse commandline and build config module

copyright (C) 2017 Koshi.Michisaka
*/

#include <stdio.h>
#include <stdlib.h>
#include <getopt.h>
#include <string.h>
#include <arpa/inet.h>
#include <sys/socket.h>

#include "config.h"

#define DEFAULT_LOCAL_PORT 60000

static void usage();

void build_config(int argc, char **argv, config *config)
{
  struct option option_info[] = {
    { "bind",   required_argument, NULL, 'b' },
    { "server", required_argument, NULL, 's' },
    { NULL,     0,                 NULL,  0  }
  };
  int opt;
  char *pos;

  memset(config, 0, sizeof(&config));

  while ((opt = getopt_long(argc, argv, "b:s:", option_info, NULL)) != -1) {
    switch (opt) {
    case 'b':
      pos = strchr(optarg, ':');
      if (pos != NULL) {
	config->bind_port = htons((int)strtol(pos + 1, (char **)NULL, 10));
	*pos = '\0';
	switch (inet_pton(AF_INET, optarg, &config->bind_address)) {
	case -1:
	  perror("inet_pton error");
	  exit(EXIT_FAILURE);
	  break;
	case 0:
	  usage();
	  exit(EXIT_FAILURE);
	}
      } else {
	config->bind_port = htons((int)strtol(optarg, (char **)NULL, 10));
      }
      break;
    case 's':
      pos = strchr(optarg, ':');
      if (pos != NULL) {
	config->server_port = htons((int)strtol(pos + 1, (char **)NULL, 10));
	*pos = '\0';
      }
      strncpy(config->server_hostname, optarg, sizeof(config->server_hostname) - 1);
      break;
    default:
      usage();
      exit(EXIT_FAILURE);
    }
  }
  argc -= optind;
  argv += optind;

  if (config->bind_address == 0) {
    config->bind_address = htonl(INADDR_ANY);
  }
  if (config->bind_port == 0) {
    config->bind_port = htons(DEFAULT_LOCAL_PORT);
  }

  if (strlen(config->server_hostname) == 0) {
    fprintf(stderr, "-s option required\n");
    usage();
    exit(EXIT_FAILURE);
  }

  if (config->server_port == 0) {
    config->server_port = config->bind_port;
  }

  return;
}

static void usage()
{
  fprintf(stderr, "usage\n");
  fprintf(stderr, "-b --bind [ADDRESS:]PORT\n");
  fprintf(stderr, "-s --server HOSTNAME[:PORT]\n");

  return;
}

