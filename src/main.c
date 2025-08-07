#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/wait.h>
#include <unistd.h>

#define HEAP_BUFFER_SIZE 256
#define STACK_BUFFER_SIZE 128
#define HISTORY_PATH_SIZE 128
#define likely(x) __builtin_expect(!!(x), 1)
#define unlikely(x) __builtin_expect(!!(x), 0)

static inline char *readline(void) {
  char stack_buffer[STACK_BUFFER_SIZE];
  size_t position = 0;
  int character;

  while ((character = getchar()) != EOF && character != '\n') {
    if (likely(position < sizeof(stack_buffer) - 1)) {
      stack_buffer[position++] = (char)character;
    } else {
      char *heap_buffer = malloc(HEAP_BUFFER_SIZE);

      if (unlikely(!heap_buffer)) {
        return NULL;
      }

      memcpy(heap_buffer, stack_buffer, sizeof(stack_buffer));

      size_t buffer_size = HEAP_BUFFER_SIZE;
      while ((character = getchar()) != EOF && character != '\n') {
        heap_buffer[position++] = (char)character;
        if (unlikely(position >= buffer_size - 1)) {
          buffer_size += HEAP_BUFFER_SIZE;
          char *new_buffer = realloc(heap_buffer, buffer_size);
          if (unlikely(!new_buffer)) {
            free(heap_buffer);
            return NULL;
          }
          heap_buffer = new_buffer;
        }
      }
      heap_buffer[position] = '\0';
      return heap_buffer;
    }
  }

  if (unlikely(position == 0 && character == EOF)) {
    return NULL;
  }

  char *result = malloc(position + 1);
  if (unlikely(!result)) {
    return NULL;
  }
  memcpy(result, stack_buffer, position);
  result[position] = '\0';
  return result;
}

static inline char **parse_line(char *line) {
  // if (unlikely(!line)) {
  //   return NULL;
  // }
  // already handled in main()

  size_t buffer_size = 8;
  size_t position = 0;
  char **tokens = malloc(buffer_size * sizeof(char *));
  if (unlikely(!tokens)) {
    return NULL;
  }

  char *token = strtok(line, " \t\r\n");
  while (likely(token != NULL)) {
    tokens[position++] = token;

    if (unlikely(position >= buffer_size)) {
      buffer_size <<= 1;
      char **new_tokens = realloc(tokens, buffer_size * sizeof(char *));
      if (unlikely(!new_tokens)) {
        free(tokens);
        return NULL;
      }
      tokens = new_tokens;
    }

    token = strtok(NULL, " \t\r\n");
  }

  tokens[position] = NULL;
  return tokens;
}

static inline int execute_process(char **args) {
  // if (unlikely(!args || !args[0])) {
  //   return 0;
  // }
  // already handled in main

  pid_t pid = fork();
  if (unlikely(pid == 0)) {
    execvp(args[0], args);
    _exit(EXIT_FAILURE);
  }
  if (likely(pid > 0)) {
    int status;
    while (waitpid(pid, &status, 0) == -1) {
      __builtin_ia32_pause();
    }
  }

  return 0;
}

static inline int cd(char **args) {
  const char *dir = (args && args[1]) ? args[1] : getenv("HOME");
  if (likely(dir)) {
    chdir(dir);
  }
  return 0;
}

static inline int exit_(void) { return 1; }

static inline int shell_dispatch(char **args) {
  if (unlikely(!args || !args[0])) {
    return 0;
  }

  const char *cmd = args[0];

  switch (cmd[0]) {
  case 'c':
    if (likely(cmd[1] == 'd' && cmd[2] == '\0')) {
      return cd(args);
    }
    break;
  case 'e':
    if (likely(cmd[1] == 'x' && cmd[2] == 'i' && cmd[3] == 't' &&
               cmd[4] == '\0')) {
      return exit_();
    }
    break;
  default:
    return execute_process(args);
  }

  return 0;
}

int main(void) {
  signal(SIGINT, SIG_IGN);

  char *line;
  char **args;
  char history_path[HISTORY_PATH_SIZE];
  int status = 0;
  snprintf(history_path, sizeof(history_path), "%s/%s", getenv("HOME"),
           ".ash_history");
  FILE *write_target = fopen(history_path, "a");

  if (unlikely(write_target == NULL)) {
    perror("Failed to open WRITE_TARGET for write_history :(");
    return 1;
  }

  setvbuf(stdout, NULL, _IOLBF, 0);

  while (likely(!status)) {
    write(STDOUT_FILENO, "> ", 2);
    line = readline();
    if (unlikely(!line)) {
      break;
    }

    fprintf(write_target, "%s\n", line);
    args = parse_line(line);
    if (likely(args)) {
      status = shell_dispatch(args);
      free(args);
    }

    free(line);
  }

  fclose(write_target);

  return 0;
}
