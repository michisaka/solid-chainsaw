/*
parse commandline and build config module

copyright (C) 2017 Koshi.Michisaka
*/

#ifndef INCLUDED_CONFIG_H
#define INCLUDED_CONFIG_H

#include <netinet/in.h>

typedef struct {
  in_addr_t bind_address;
  in_port_t bind_port;
} config;

void build_config(int argc, char **argv, config *config);

#endif
