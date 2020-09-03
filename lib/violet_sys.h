// 
// The Violet Project
// 
// Copyright (c) 2019, Johns Hopkins University - Order Lab.
//
//    All rights reserved.
//    Licensed under the Apache License, Version 2.0 (the "License");
//

#ifndef __VIOLET_SYS_H_
#define __VIOLET_SYS_H_

#ifdef HAVE_VIOLET_S2E
// include s2e header file if we are building the binary for s2e
#include <s2e/s2e.h>
#endif

#include <stdlib.h>
#include <stdio.h>
#include <stdarg.h>
#include <stdbool.h>
#include <string.h>

#define VIOLET_CONFIGS_ENV_NAME "VIO_SYM_CONFIGS"

#ifdef __cplusplus
extern "C" {
#endif

typedef struct {
  const char *config_blacklist;
  const char *log_file_name;
  const char *related_config_file_name;
  const char *config_meta_file_name;
} violet_init_args;

void violet_init(violet_init_args args);
void violet_done();
void violet_log(const char *msg, ...);
void violet_close_log();

void violet_parse_config_targets();
void get_related_configs(char *config_targets);
bool is_config_in_targets(const char *name);
bool is_blacklisted(const char *name);
void my_s2e_make_symbolic(void *buf, size_t size, const char *name);

extern FILE *violet_config_meta_file;

#ifdef __cplusplus
}  // extern "C"
#endif

#endif /* __VIOLET_SYS_H_ */
