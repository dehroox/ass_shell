
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define string char * // this is probably malpractice but i likey

#define READLINE_BUFFER_SIZE 128
static string readline(void) {
  uint16_t buffer_size = READLINE_BUFFER_SIZE;
  uint16_t current_position = 0;
  string buffer = malloc(sizeof(char) * (size_t)buffer_size);
  int character;

  do {
    character = getchar();

    // store character or null terminator
    buffer[current_position] =
        (char)((~((character == EOF) | (character == '\n')) & character) |
               (((character == EOF) | (character == '\n')) & '\0'));

    // increment current_position only for valid characters
    current_position += ~((character == EOF) | (character == '\n')) & 1;

    bool need_realloc = (current_position >= buffer_size);
    buffer_size += READLINE_BUFFER_SIZE * (uint8_t)need_realloc;
    string new_buffer = realloc(buffer, (size_t)buffer_size);
    uintptr_t success_mask = (uintptr_t)(new_buffer != NULL);
    buffer = (string)(((uintptr_t)new_buffer & success_mask) |
                      ((uintptr_t)buffer & ~success_mask));

  } while ((character != EOF) & (character != '\n'));

  return buffer;
}
#undef READLINE_BUFFER_SIZE

#define TOKEN_BUFFER_SIZE 64
#define TOKEN_DELIMETERS "\t\r\n\a"
static string *parse_line(string line) {
  uint16_t buffer_size = TOKEN_BUFFER_SIZE;
  uint16_t current_position = 0;
  string *tokens =
      (string *)malloc((unsigned long)buffer_size * sizeof(string));
  string token;
  bool is_null;

  token = strtok(line, TOKEN_DELIMETERS);
  do {
    is_null = (token == NULL);
    tokens[current_position] = token;
    current_position += !is_null;

    bool need_realloc = (current_position >= buffer_size);
    buffer_size += TOKEN_BUFFER_SIZE * (uint8_t)need_realloc;

    string *new_tokens =
        (string *)realloc((void *)tokens, buffer_size * sizeof(char *));
    uintptr_t success_mask = (uintptr_t)(new_tokens != NULL);
    tokens = (string *)(((uintptr_t)new_tokens & success_mask) |
                        ((uintptr_t)tokens & ~success_mask));

    token = strtok(NULL, TOKEN_DELIMETERS);
  } while (!is_null);

  return tokens;
}
#undef TOKEN_BUFFER_SIZE
#undef TOKEN_DELIMETERS

int main(void) {
  string line = NULL;
  string *args;
  int status = 0;

  do {
    printf("> ");
    line = readline();
    args = parse_line(line);

    free(line);
    free((void *)args);
  } while (!status);
  return 0;
}
