#include <signal.h>
#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

/**
 * tee [ -a ] [ -i ] [ File ... ]
 *
 * A little implementation of GNU coreutils tee.
 *
 * Copy standard input to each FILE, and also to standard output.
 *
 * -a to append to files
 * -i to ignore interrupt signals
 *
 * TODO: more options for handling errors (see --output-error in GNU tee).
 * TODO: testing
 * TODO: manual buffering
 * TODO: use const
 * TODO: use errors.h functions
 */

/**
 * Prints a formatted message to stderr, prints a friendly version of errno, and then exits with error code 1.
 */
void errorf(char *fmt, ...) {
  va_list args;
  va_start(args, fmt);
  vfprintf(stderr, fmt, args);
  va_end(args);
  perror(NULL);
  exit(1);
}

/**
 * Opens given filenames with provided mode and store the file pointer in the provided array.
 */
void open_files(char *filenames[], char *mode, FILE *file_pointers[], int n) {
  int i;
  for (i = 0; i < n; i++) {
    file_pointers[i] = fopen(filenames[i], mode);
    if (file_pointers[i] == NULL) {
      errorf("%s could not be opened with mode %s\n", filenames[i], mode);
    }
  }
}

/**
 * Streams stdin to n file pointers.
 */
void stream_files(FILE *file_pointers[], int n) {
  int b, i;
  // This should be buffered... is there a generally "good" constant?
  while ((b = getchar()) != EOF) {
    for (i = 0; i < n; i++) {
      if (fputc(b, file_pointers[i]) == EOF) {
        errorf("Failed writing to %s\n", file_pointers[i]);
      }
    }
  }
}

/**
 * Closes n file pointers, ignoring any errors.
 */
void close_files(FILE *file_pointers[], int n) {
  int i;
  for (i = 0; i < n; i++) {
    fclose(file_pointers[i]);
  }
}

/**
 * Registers a handler for SIGINT that ignores the signal.
 */
void ignore_sigint() {
  struct sigaction action;
  action.sa_handler = SIG_IGN; // Handler to ignore signal.
  sigaction(SIGINT, &action, NULL);
}

/**
 * TODO: testing
 * Some techniques that seem promising if I wanted to spend more time on testing:
 * - Redirecting stdin and stdout with `freopen` for the core of the program
 * - Using the same `freopen` technique to test appending
 * - Using `raise` to test ignoring interrupts
 */
void test() {}

int main(int argc, char *argv[]) {
  char opt;
  char *mode = "w";
  FILE *file_pointers[argc + 1];

  while ((opt = getopt(argc, argv, "ai")) != EOF) {
    switch(opt) {
    case 'a':
      mode = "a";
      break;
    case 'i':
      ignore_sigint();
      break;
    default:
      errorf("Unknown option %c\n", opt);
    }
  }

  argc -= optind;
  argv += optind;

  file_pointers[0] = stdout; // Always include stdout.
  open_files(argv, mode, file_pointers + 1, argc);
  stream_files(file_pointers, argc + 1);
  close_files(file_pointers + 1, argc); // Don't close stdout.
  return 0;
}

