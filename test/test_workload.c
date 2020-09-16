#include <stdio.h>
#include "violet_sys.h"

int main(int argc, char* argv[]) {
  violet_init_args args;
  memset(&args, 0, sizeof(args));
  violet_init(args);
  if (argc == 2) {
    char* request = NULL;
    generate_one_symbolic_request(&request, argv[1]);
    if (request != NULL) violet_log("generated on request: %s\n", request);
  } else if (argc == 3) {
    char* requests[MAX_SYMBOLIC_REQUEST_TYPE];
    int requests_len =
        generate_multi_symbolic_requests(requests, argv[1], argv[2]);
    if (requests_len > 0) {
      for (int i = 0; i < requests_len; i++) {
        violet_log("generated request %d: %s\n", i + 1, requests[i]);
      }
    }
  } else {
    fprintf(stderr, "Usage: %s <options_file> <workload_template_file>\n",
            argv[0]);
  }
  violet_done();
  return 0;
}
