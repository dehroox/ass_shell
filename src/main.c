#include <signal.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/wait.h>
#include <unistd.h>

#define string char *

#define READLINE_BUFFER_SIZE 256
static string readline(void) {
  int buffer_size = READLINE_BUFFER_SIZE;
  int position = 0;
  string buffer = malloc(sizeof(char) * (size_t)buffer_size);
  int character;

  do {
    character = getchar();
    buffer[position] =
        (char)((~((character == EOF) | (character == '\n')) & character) |
               (((character == EOF) | (character == '\n')) & '\0'));
    position += (~((character == EOF) | (character == '\n'))) & 1;
    int need_realloc = (position >= buffer_size - 1);
    buffer_size += READLINE_BUFFER_SIZE * need_realloc;
    string new_buffer = realloc(buffer, (size_t)buffer_size);
    uintptr_t success_mask = (uintptr_t)(new_buffer != NULL);
    buffer = (string)(((uintptr_t)new_buffer & success_mask) |
                      ((uintptr_t)buffer & ~success_mask));
  } while ((character != EOF) & (character != '\n'));

  return buffer;
}
#undef READLINE_BUFFER_SIZE

#define TOKEN_BUFFER_SIZE 64
#define TOKEN_DELIMETERS " \t\r\n\a"
static string *parse_line(string line) {
  int buffer_size = TOKEN_BUFFER_SIZE;
  int position = 0;
  string *tokens =
      (string *)malloc((unsigned long)buffer_size * sizeof(string));
  string token;
  int is_null;

  token = strtok(line, TOKEN_DELIMETERS);
  do {
    is_null = (token == NULL);
    tokens[position] = token;
    position += !is_null;
    int need_realloc = (position >= buffer_size - 1);
    buffer_size += TOKEN_BUFFER_SIZE * need_realloc;
    string *new_tokens = (string *)realloc(
        (void *)tokens, (unsigned long)buffer_size * sizeof(string));
    uintptr_t success_mask = (uintptr_t)(new_tokens != NULL);
    tokens = (string *)(((uintptr_t)new_tokens & success_mask) |
                        ((uintptr_t)tokens & ~success_mask));
    token = strtok(NULL, TOKEN_DELIMETERS);
  } while (!is_null);

  return tokens;
}
#undef TOKEN_BUFFER_SIZE
#undef TOKEN_DELIMETERS

static int execute_process(string *args) {
  pid_t pid = fork();
  if (pid == 0) {
    execvp(args[0], args);
    exit(1);
  } else if (pid > 0) {
    int status;
    waitpid(pid, &status, 0);
  }
  return 0;
}

static int cd(string *args) {
  const char *dir = (args && args[1]) ? args[1] : getenv("HOME");
  if (dir) {
    chdir(dir);
  }
  return 0;
}

static int exit_() { return 1; }

static int shell_dispatch(string *args) {
  if (!args || !args[0]) {
    return 0;
  }
  string cmd = args[0];
  int len = 0;
  while (cmd[len]) {
    len++;
  }

  if (cmd[0] == 'c' && cmd[1] == 'd' && cmd[2] == '\0') {
    return cd(args);
  }
  if (cmd[0] == 'e' && cmd[1] == 'x' && cmd[2] == 'i' && cmd[3] == 't' &&
      cmd[4] == '\0') {
    return exit_();
  }

  return execute_process(args);
}

int main(void) {
  (void)signal(SIGINT, SIG_IGN);
  string line;
  string *args;
  int status = 0;

  do {
    printf("> ");
    line = readline();
    args = parse_line(line);
    status = shell_dispatch(args);
    free(line);
    free((void *)args);
  } while (!status);

  return 0;
}
