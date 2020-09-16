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
const char *sym_config_blacklist = "";

FILE *violet_log_file = NULL;
const char *log_file_name = "violet_unknown_sys.log";

FILE *violet_related_config_file = NULL;
const char *related_config_file_name = "related_configuration.log";

FILE *violet_config_meta_file = NULL;
const char *config_meta_file_name = "configurations.log";

static const char *json_configuration_message =
    "Json Configuration is invalid.";

void violet_init(violet_init_args args) {
  if (args.config_blacklist != NULL)
    sym_config_blacklist = args.config_blacklist;
  if (args.log_file_name != NULL) log_file_name = args.log_file_name;
  if (args.related_config_file_name != NULL)
    related_config_file_name = args.related_config_file_name;
  if (args.config_meta_file_name != NULL)
    config_meta_file_name = args.config_meta_file_name;
  violet_config_meta_file = fopen(config_meta_file_name, "w");
  violet_log_file = fopen(log_file_name, "w");
#ifdef HAVE_LIBVIOLET_S2E
  s2e_printf("done with violet init within S2E\n");
#else
  printf("done with violet init\n");
#endif
}

void violet_done() {
  violet_close_log();
  if (violet_config_meta_file != NULL) {
    fclose(violet_config_meta_file);
    violet_config_meta_file = NULL;
  }
}

void violet_log(const char *msg, ...) {
  char buf[512];
  va_list args;
  va_start(args, msg);
  vsnprintf(buf, sizeof(buf), msg, args);
  va_end(args);
#ifdef HAVE_LIBVIOLET_S2E
  // if we are executing in S2E, call s2e_printf
  s2e_printf("%s", buf);
#else
  if (violet_log_file == NULL) {
    violet_log_file = stderr;
  }
  // print to both console and log file
  if (violet_log_file != stderr && violet_log_file != stdout) {
    fprintf(stderr, "%s", buf);
  }
  fprintf(violet_log_file, "%s", buf);
  fflush(violet_log_file);
#endif
}

void violet_close_log() {
#ifndef HAVE_LIBVIOLET_S2E
  if (violet_log_file != NULL && violet_log_file != stdout &&
      violet_log_file != stderr) {
    fclose(violet_log_file);
    violet_log_file = NULL;
  }
#endif
}

bool is_config_in_targets(const char *name) {
  if (sym_config_targets_len <= 0 || name == NULL) {
    return false;
  }
  if (sym_config_targets_all) {  // match all, will be too much...
    return true;
  }
  size_t namelen = strlen(name);
  if (namelen > sym_config_targets_len) {
    return false;
  }
  const char *p = strstr(sym_config_targets, name);
  while (p != NULL) {
    p--;
    if (*p != ',' && *p != ' ' && *p != '\0') {
      // indicates the name is part of the comma or space separated list
      // reposition p and break out of the loop
      p += namelen + 1;
      p = strstr(p, name);
      continue;
    }
    p += namelen + 1;
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

void my_s2e_make_symbolic(void *buf, size_t size, const char *name) {
  if (is_blacklisted(name)) {
    return;
  }
  if (size > 100) {
    violet_log(
        "WARNING: trying to make a large chunk of memory %s symbolic: %lu "
        "bytes starting at %p\n",
        name, size, buf);
  } else {
    violet_log("will make %s symbolic: %lu bytes starting at %p\n", name, size,
               buf);
  }
#ifdef HAVE_LIBVIOLET_S2E
  s2e_make_symbolic(buf, size, name);
  s2e_printf("actually called S2E to make %s symbolic!!!!\n", name);
#else
  printf("not in S2E engine, skip making %s symbolic\n", name);
#endif
}

bool is_blacklisted(const char *name) {
  size_t namelen = strlen(name);
  size_t blacklistlen = strlen(sym_config_blacklist);
  if (namelen > blacklistlen) {
    return false;
  }
  const char *p = strstr(sym_config_blacklist, name);
  while (p != NULL) {
    p--;
    if (*p != ',' && *p != ' ' && *p != '\0') {
      // indicates the name is part of the comma or space separated list
      // reposition p and break out of the loop
      p += namelen + 1;
      p = strstr(p, name);
      continue;
    }
    p += namelen + 1;
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
  char *rest = (char *)malloc(strlen(config_targets) * sizeof(char));
  char *target_config, *related_configurations, *line = NULL;
  size_t len = 0;

  strcpy(rest, config_targets);
  target_config = strtok_r(rest, s, &rest);
  while (target_config != NULL) {
    violet_related_config_file = fopen(related_config_file_name, "r");
    if (violet_related_config_file == NULL) {
      fprintf(stderr, "related configuration file %s does not exist\n",
              related_config_file_name);
      break;
    }
    while ((getline(&line, &len, violet_related_config_file)) != -1) {
      char *config, *temp;
      config = strtok_r(line, "[", &related_configurations);
      related_configurations = strtok(related_configurations, "]");
      if (strstr(related_configurations, "\n")) continue;
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
  if (violet_related_config_file != NULL) fclose(violet_related_config_file);
  free(rest);
}

void violet_parse_config_targets() {
  char *temp = getenv(VIOLET_CONFIGS_ENV_NAME);
  if (temp != NULL) {
    strcpy(sym_config_targets, temp);
    violet_log("The sym_config_targets is %s\n", sym_config_targets);
#ifdef HAVE_LIBVIOLET_S2E
    s2e_invoke_plugin("LatencyTracker", &sym_config_targets,
                      sizeof(sym_config_targets));
#endif
  }
  if (sym_config_targets == NULL || strlen(sym_config_targets) == 0) {
    sym_config_targets_len = 0;
    violet_log("%s environment variable is not set\n", VIOLET_CONFIGS_ENV_NAME);
  } else {
    if (strcmp(sym_config_targets, "*") == 0) {
      sym_config_targets_all = true;  // wildcard match all configs
      violet_log("will make all configs symbolic; are you sure?\n");
    } else {
      violet_log("will make the following configs symbolic: %s\n",
                 sym_config_targets);
    }
    if (!sym_config_targets_all) {
      // get related configs if the target config is non-empty and not a
      // wildcard
      get_related_configs(sym_config_targets);
    }
    sym_config_targets_len = strlen(sym_config_targets);
    violet_log("finish checking the result configuration: %s\n",
               sym_config_targets);
  }
}

bool read_workload_json(const char *json_file, char **buf, size_t *len) {
  FILE *fjson = fopen(json_file, "r");
  if (!fjson) {
    perror("Cannot open json file");
    return false;
  }
  fseek(fjson, 0, SEEK_END);
  long int sz = ftell(fjson);
  rewind(fjson);
  char *data = (char *)malloc((sz + 1) * sizeof(char));
  if (data == NULL) {
    fprintf(stderr, "Failed to allocate buffer to read file %s\n", json_file);
    return false;
  }
  size_t readlen = fread(data, 1, sz, fjson);
  if (ferror(fjson)) {
    fprintf(stderr, "Failed to read file %s\n", json_file);
    free(data);
    return false;
  }
  data[readlen++] = '\0';
  *buf = data;
  *len = readlen;
  return true;
}

int get_workload_helper(cJSON *current_level, int **workload_options) {
  if (!cJSON_IsObject(current_level)) {
#ifdef HAVE_VIOLET_S2E
    s2e_kill_state(1, "workload has no options, probably reached last level");
#else
    fprintf(stderr,
            "workload '%s' has no options, probably reached last level\n",
            current_level->valuestring);
    return -1;
#endif
  }
  cJSON *options = cJSON_GetObjectItemCaseSensitive(current_level, "options");
  if (!cJSON_IsArray(options)) {
#ifdef HAVE_VIOLET_S2E
    s2e_kill_state(1, json_configuration_message);
#else
    fprintf(stderr, "%s: '%s'\n", json_configuration_message,
            options->valuestring);
    return -1;
#endif
  }
  int size = cJSON_GetArraySize(options);
  if (size == 0) {
#ifdef HAVE_VIOLET_S2E
    s2e_kill_state(1, json_configuration_message);
#else
    fprintf(stderr, "%s: '%s'\n", json_configuration_message,
            options->valuestring);
    return -1;
#endif
  }
  int index = 0;
#ifdef HAVE_VIOLET_S2E
  s2e_make_symbolic(&index, sizeof(int), "index");
  s2e_assume(index >= 0 && index < size);
#endif
  cJSON *option = cJSON_GetArrayItem(options, index);
  if (!cJSON_IsString(option)) {
#ifdef HAVE_VIOLET_S2E
    s2e_kill_state(1, "Workload option item is not a string");
#else
    fprintf(stderr, "Workload option item '%s' is not a string\n",
            option->valuestring);
    return -1;
#endif
  }
  char *option_string = option->valuestring;
  violet_log("workload option string: %s\n", option_string);
  cJSON *next_level =
      cJSON_GetObjectItemCaseSensitive(current_level, option_string);
  if (cJSON_IsArray(next_level)) {
    int array_size = cJSON_GetArraySize(next_level);
    (*workload_options) = (int *)malloc(sizeof(int) * array_size);
    for (int i = 0; i < array_size; i++) {
      cJSON *pSub = cJSON_GetArrayItem(next_level, i);
      if (NULL == pSub) {
        continue;
      }
      int ivalue = pSub->valueint;
      (*workload_options)[i] = ivalue;
    }
    return array_size;
  }
  return get_workload_helper(next_level, workload_options);
}

char *get_symbolic_request_helper(cJSON *current_level) {
  if (!cJSON_IsObject(current_level)) {
#ifdef HAVE_VIOLET_S2E
    s2e_kill_state(1, "workload has no options, probably reached last level");
#else
    fprintf(stderr,
            "workload '%s' has no options, probably reached last level\n",
            current_level->valuestring);
    return NULL;
#endif
  }
  cJSON *options = cJSON_GetObjectItemCaseSensitive(current_level, "options");
  if (!cJSON_IsArray(options)) {
#ifdef HAVE_VIOLET_S2E
    s2e_kill_state(1, json_configuration_message);
#else
    fprintf(stderr, "%s: '%s'\n", json_configuration_message,
            options->valuestring);
    return NULL;
#endif
  }
  int size = cJSON_GetArraySize(options);
  if (size == 0) {
#ifdef HAVE_VIOLET_S2E
    s2e_kill_state(1, json_configuration_message);
#else
    fprintf(stderr, "%s: '%s'\n", json_configuration_message,
            options->valuestring);
    return NULL;
#endif
  }
  int index = 0;
#ifdef HAVE_VIOLET_S2E
  s2e_make_symbolic(&index, sizeof(int), "index");
  s2e_assume(index >= 0 && index < size);
#endif
  cJSON *option = cJSON_GetArrayItem(options, index);
  if (!cJSON_IsString(option)) {
#ifdef HAVE_VIOLET_S2E
    s2e_kill_state(1, "Workload option item is not a string");
#else
    fprintf(stderr, "Workload option item '%s' is not a string\n",
            option->valuestring);
    return NULL;
#endif
  }
  char *option_string = option->valuestring;
  cJSON *next_level =
      cJSON_GetObjectItemCaseSensitive(current_level, option_string);
  if (cJSON_IsString(next_level)) {
    return next_level->valuestring;
  }
  return get_symbolic_request_helper(next_level);
}

int get_symbolic_requests_helper(cJSON *current_level, char *workloads[],
                                 int workload_options[], int level, int depth) {
  cJSON *target_level = current_level;
  cJSON *jsons[MAX_SYMBOLIC_REQUEST_TYPE];
  int jsons_depth = 0;
  int workloads_depth = 0;

  while (level <= depth) {
    if (!cJSON_IsObject(target_level)) {
#ifdef HAVE_VIOLET_S2E
      s2e_kill_state(1, "workload has no options, probably reached last level");
#else
      fprintf(stderr,
              "workload '%s' has no options, probably reached last level\n",
              target_level->valuestring);
      return -1;
#endif
    }
    cJSON *options = cJSON_GetObjectItemCaseSensitive(target_level, "options");
    if (!cJSON_IsArray(options)) {
#ifdef HAVE_VIOLET_S2E
      s2e_kill_state(1, json_configuration_message);
#else
      fprintf(stderr, "%s: '%s'\n", json_configuration_message,
              options->valuestring);
      return -1;
#endif
    }
    int size = cJSON_GetArraySize(options);
    if (size == 0) {
#ifdef HAVE_VIOLET_S2E
      s2e_kill_state(1, json_configuration_message);
#else
      fprintf(stderr, "%s: '%s'\n", json_configuration_message,
              options->valuestring);
      return -1;
#endif
    }
    cJSON *option = cJSON_GetArrayItem(options, workload_options[level]);
    if (!cJSON_IsString(option)) {
#ifdef HAVE_VIOLET_S2E
      s2e_kill_state(1, json_configuration_message);
#else
      fprintf(stderr, "%s: '%s'\n", json_configuration_message,
              option->valuestring);
      return -1;
#endif
    }
    char *option_string = option->valuestring;
    target_level =
        cJSON_GetObjectItemCaseSensitive(target_level, option_string);
    level++;
    if (level > depth) {
      jsons[jsons_depth] = target_level;
      jsons_depth++;
    }
  }

  while (jsons_depth > 0) {
    cJSON *current_level = jsons[jsons_depth - 1];
    jsons_depth--;
    if (cJSON_IsString(current_level)) {
      (workloads)[workloads_depth] = current_level->valuestring;
      workloads_depth++;
      continue;
    }
    if (!cJSON_IsObject(current_level)) {
#ifdef HAVE_VIOLET_S2E
      s2e_kill_state(1, "workload has no options, probably reached last level");
#else
      fprintf(stderr,
              "workload '%s' has no options, probably reached last level\n",
              current_level->valuestring);
      return -1;
#endif
    }
    cJSON *options = cJSON_GetObjectItemCaseSensitive(current_level, "options");
    if (!cJSON_IsArray(options)) {
#ifdef HAVE_VIOLET_S2E
      s2e_kill_state(1, json_configuration_message);
#else
      fprintf(stderr, "%s: '%s'\n", json_configuration_message,
              options->valuestring);
      return -1;
#endif
    }
    int size = cJSON_GetArraySize(options);
    if (size == 0) {
#ifdef HAVE_VIOLET_S2E
      s2e_kill_state(1, json_configuration_message);
#else
      fprintf(stderr, "%s: '%s'\n", json_configuration_message,
              options->valuestring);
      return -1;
#endif
    }
    for (int i = 0; i < size; i++) {
      char *option_string = cJSON_GetArrayItem(options, i)->valuestring;
      jsons[jsons_depth] =
          cJSON_GetObjectItemCaseSensitive(current_level, option_string);
      jsons_depth++;
    }
  }
  return workloads_depth;
}

int generate_one_symbolic_request(char **symbolic_request,
                                  const char *workload_template_file) {
  char *workload_json_str;
  size_t workload_str_len;
  if (!read_workload_json(workload_template_file, &workload_json_str,
                          &workload_str_len)) {
    violet_log("failed to load workload template file\n");
    return -1;
  }
  cJSON *current_level = cJSON_Parse(workload_json_str);
  char *request_value = get_symbolic_request_helper(current_level);
  if (request_value == NULL) {
    violet_log("failed to get symbolic request for %s\n",
               current_level->valuestring);
    return -1;
  }
  int request_length = strlen(request_value);
  *symbolic_request = (char *)malloc(sizeof(char) * request_length);
  strcpy(*symbolic_request, request_value);
  cJSON_Delete(current_level);
  return request_length;
}

int generate_multi_symbolic_requests(char **symbolic_requests,
                                     const char *options_file,
                                     const char *workload_template_file) {
  char *options_str;
  size_t options_str_len;
  if (!read_workload_json(options_file, &options_str, &options_str_len)) {
    violet_log("failed to load options file\n");
    return -1;
  }
  char *workload_json_str;
  size_t workload_str_len;
  if (!read_workload_json(workload_template_file, &workload_json_str,
                          &workload_str_len)) {
    violet_log("failed to load workload template file\n");
    return -1;
  }
  cJSON *current_level = cJSON_Parse(options_str);
  int *workload_options, option_length;
  option_length = get_workload_helper(current_level, &workload_options);
  current_level = cJSON_Parse(workload_json_str);
  return get_symbolic_requests_helper(current_level, symbolic_requests,
                                      workload_options, 0, option_length - 1);
}
