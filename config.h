/*
parse commandline and build config module

copyright (C) 2017 Koshi.Michisaka
*/

#ifndef INCLUDED_CONFIG_H
#define INCLUDED_CONFIG_H

typedef struct config {
  char bind_address[254];
  char bind_port[17];
  char server_hostname[254];
  char server_port[17];
} config;

void build_config(int argc, char **argv, config *config);

#endif
