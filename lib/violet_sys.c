// 
// The Violet Project
// 
// Copyright (c) 2019, Johns Hopkins University - Order Lab.
//
//    All rights reserved.
//    Licensed under the Apache License, Version 2.0 (the "License");
//

#include "violet_sys.h"
#include <violet_config.h>

#ifdef HAVE_LIBVIOLET_S2E
#include <s2e/s2e.h>
#endif

char sym_config_targets[1024];
size_t sym_config_targets_len = 0;
bool sym_config_targets_all = false;
const char* sym_config_blacklist = "";

FILE *violet_log_file = NULL;
const char* violet_log_file_name = "violet_unknown_sys.log";

FILE *violet_related_config_file = NULL;
const char* violet_related_config_file_name = "related_configuration.log";

FILE *violet_config_meta_file = NULL;
const char* violet_config_meta_file_name = "configurations.log";

void violet_init(violet_init_args args)
{
  sym_config_blacklist = args.config_blacklist;
  violet_log_file_name = args.log_file_name;
  violet_related_config_file_name = args.related_config_file_name;
  violet_config_meta_file_name = args.config_meta_file_name;

  if(violet_config_meta_file == NULL) {
    violet_config_meta_file = fopen(violet_config_meta_file_name, "w");
  }
#ifdef HAVE_LIBVIOLET_S2E
  s2e_printf("Done with violet init\n");
#endif
}

void violet_done()
{
  violet_close_log();
  if (violet_config_meta_file != NULL) {
    fclose(violet_config_meta_file);
    violet_config_meta_file = NULL;
  }
}

void violet_log(const char *msg, ...)
{
  char buf[512];
  va_list args;
  va_start(args, msg);
  vsnprintf(buf, sizeof(buf), msg, args);
  va_end(args);
#ifdef HAVE_VIOLET_S2E
  // if we are executing in S2E, call s2e_printf
  // s2e_message(buf);
#else
  if (violet_log_file == NULL) {
    violet_log_file = fopen(violet_log_file_name, "w");
    if (violet_log_file == NULL) {
      violet_log_file = stderr;
    }
  }
  // print to both console and log file
  if (violet_log_file != stderr && violet_log_file != stdout) {
    fprintf(stderr, "%s", buf);
  }
  fprintf(violet_log_file, "%s", buf);
  fflush(violet_log_file);
#endif
}

void violet_close_log()
{
#ifndef HAVE_VIOLET_S2E
  if (violet_log_file != NULL && violet_log_file != stdout && 
      violet_log_file != stderr) {
    fclose(violet_log_file);
    violet_log_file = NULL;
  }
#endif
}

bool is_config_in_targets(const char *name)
{
  if (sym_config_targets_len <= 0 || name == NULL) {
    return false;
  }
  if (sym_config_targets_all) { // match all, will be too much...
    return true;
  }
  size_t namelen = strlen(name);
  if (namelen > sym_config_targets_len) {
    return false;
  }
  const char *p = strstr(sym_config_targets, name);
  while (p != NULL) {
    p --;
    if (*p != ',' && *p != ' ' && *p != '\0') {
      // indicates the name is part of the comma or space separated list
      // reposition p and break out of the loop
      p += namelen+1;
      p = strstr(p, name);
      continue;
    }
    p += namelen+1;
    if (*p == ',' || *p == ' ' || *p == '\0') {
      // indicates the name is part of the comma or space separated list
      // reposition p and break out of the loop
      p -= namelen;
      break;
    }
    // just matched part of a large string name, keep searching
    p = strstr(p, name);
  }
  return p != NULL;
}

void my_s2e_make_symbolic(void *buf, size_t size, const char *name)
{
  if(is_blacklisted(name)) {
    return;
  }
  if (size > 100) {
    violet_log("WARNING: trying to make a large chunk of memory %s symbolic: %lu bytes starting at %p\n", name, size, buf);
  } else {
    violet_log("will make %s symbolic: %lu bytes starting at %p\n", name, size, buf);
  }
#ifdef HAVE_VIOLET_S2E
  s2e_make_symbolic(buf, size, name);
  s2e_printf("actually called S2E to make %s symbolic!!!!\n", name);
#endif 
}

bool is_blacklisted(const char *name)
{
  size_t namelen = strlen(name);
  size_t blacklistlen = strlen(sym_config_blacklist);
  if (namelen > blacklistlen) {
    return false;
  }
  const char *p = strstr(sym_config_blacklist, name);
  while (p != NULL) {
    p --;
    if (*p != ',' && *p != ' ' && *p != '\0') {
      // indicates the name is part of the comma or space separated list
      // reposition p and break out of the loop
      p += namelen+1;
      p = strstr(p, name);
      continue;
    }
    p += namelen+1;
    if (*p == ',' || *p == ' ' || *p == '\0') {
      // indicates the name is part of the comma or space separated list
      // reposition p and break out of the loop
      p -= namelen;
      break;
    }
    // just matched part of a large string name, keep searching
    p = strstr(p, name);
  }
  return p != NULL;
}

void get_related_configs(char *config_targets) {
  const char s[2] = ";";
  char *rest = (char*) malloc(strlen(config_targets) * sizeof(char));
  char *target_config, *related_configurations, *line = NULL;
  size_t len = 0;

  strcpy(rest, config_targets);
  target_config = strtok_r(rest, s, &rest);
  while (target_config != NULL) {
    violet_related_config_file = fopen(violet_related_config_file_name, "r");
    if (violet_related_config_file == NULL) {
      fprintf(stderr, "related configuration file %s does not exist\n", 
          violet_related_config_file_name);
      break;
    }
    while ((getline(&line, &len, violet_related_config_file)) != -1) {
      char *config, *temp;
      config = strtok_r(line, "[", &related_configurations);
      related_configurations = strtok(related_configurations, "]");
      if(strstr(related_configurations, "\n"))
        continue;
      config = strtok_r(config, ",", &temp);
      strtok(config, " ");
      strtok(NULL, " ");
      config = strtok(NULL, " ");
      if (!strcmp(config, target_config)) {
        strcat(config_targets, ",");
        strcat(config_targets, related_configurations);
      }
    }
    target_config = strtok_r(rest, s, &rest);
  }
  if (violet_related_config_file != NULL)
    fclose(violet_related_config_file);
}

void violet_make_configs_symbolic() 
{
  char* temp = getenv(VIOLET_CONFIGS_ENV_NAME);
  if (temp != NULL) {
    strcpy(sym_config_targets, temp);
    violet_log("The sym_config_targets is %s\n", sym_config_targets);
#ifdef HAVE_VIOLET_S2E
    s2e_invoke_plugin("LatencyTracker", &sym_config_targets, sizeof(sym_config_targets));
#endif
  }
  if (sym_config_targets == NULL || strlen(sym_config_targets) == 0) {
    violet_log("%s environment variable is not set\n", VIOLET_CONFIGS_ENV_NAME);
  } else {
    sym_config_targets_len = strlen(sym_config_targets);
    if (sym_config_targets_len > 0 && sym_config_targets_all) {
      // get related configs if the target config is non-empty and not a wildcard
      get_related_configs(sym_config_targets);
    }
    sym_config_targets_len = strlen(sym_config_targets);
    violet_log("finish checking the result configuration: %s\n", sym_config_targets);
    if (strcmp(sym_config_targets, "*") == 0) {
      sym_config_targets_all = true;  // wildcard match all configs
      violet_log("will make all configs symbolic; are you sure?\n");
    } else {
      violet_log("will make the following configs symbolic: %s\n", sym_config_targets);
    }
  }
}
