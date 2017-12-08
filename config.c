/*
parse commandline and build config module

copyright (C) 2017 Koshi.Michisaka
*/

#include <stdio.h>
#include <stdlib.h>
#include <getopt.h>
#include <string.h>

#include "config.h"

#define DEFAULT_LOCAL_PORT "60000"

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

  memset(config, 0, sizeof(struct config));

  while ((opt = getopt_long(argc, argv, "b:s:", option_info, NULL)) != -1) {
    switch (opt) {
    case 'b':
      pos = strchr(optarg, ':');
      if (pos != NULL) {
	*pos = '\0';
	strncpy(config->bind_address, optarg, sizeof(config->bind_address) - 1 );
	strncpy(config->bind_port, pos + 1, sizeof(config->bind_port) - 1);
      } else {
	strncpy(config->bind_port, optarg, sizeof(config->bind_port) - 1);
      }
      break;
    case 's':
      pos = strchr(optarg, ':');
      if (pos != NULL) {
	*pos = '\0';
	strncpy(config->server_port, pos + 1, sizeof(config->server_port) - 1);
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

  if (strlen(config->bind_port) == 0) {
    strncpy(config->bind_port, DEFAULT_LOCAL_PORT, sizeof(config->bind_port) - 1);
  }

  if (strlen(config->server_hostname) == 0) {
    fprintf(stderr, "-s option required\n");
    usage();
    exit(EXIT_FAILURE);
  }

  if (strlen(config->server_port) == 0) {
    strncpy(config->server_port, config->bind_port, sizeof(config->server_port));
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

