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

#include <cJSON.h>

#include <stdarg.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define VIOLET_CONFIGS_ENV_NAME "VIO_SYM_CONFIGS"

#ifdef __cplusplus
extern "C" {
#endif

// FIXME
#define MAX_SYMBOLIC_REQUEST_TYPE 10

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

int generate_one_symbolic_request(char **symbolic_request,
                                  const char *workload_template_file);
int generate_multi_symbolic_requests(char **symbolic_requests,
                                     const char *options_file,
                                     const char *workload_template_file);
char *get_symbolic_request_helper(cJSON *current_level);
int get_symbolic_requests_helper(cJSON *current_level, char *workloads[],
                                 int workload_options[], int level, int depth);
int get_workload_helper(cJSON *current_level, int **workload_options);
bool read_workload_json(const char *json_file, char **buf, size_t *len);

extern FILE *violet_config_meta_file;

#ifdef __cplusplus
}  // extern "C"
#endif

#endif /* __VIOLET_SYS_H_ */
