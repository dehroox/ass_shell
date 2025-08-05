
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define string char *

#define READLINE_BUFFER_SIZE 128
static string readline(void) {
  int buffer_size = READLINE_BUFFER_SIZE;
  int current_position = 0;
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

    int need_realloc = (current_position >= buffer_size);
    buffer_size += READLINE_BUFFER_SIZE * need_realloc;
    string new_buffer = realloc(buffer, (size_t)buffer_size);

    memcpy(buffer, new_buffer,
           sizeof(string) * (size_t)(need_realloc & (new_buffer != NULL)));
  } while ((character != EOF) & (character != '\n'));

  return buffer;
}
#undef READLINE_BUFFER_SIZE

#define TOKEN_BUFFER_SIZE 64
#define TOKEN_DELIMETERS "\t\r\n\a"
static string *parse_line(string line) {
  int buffer_size = TOKEN_BUFFER_SIZE;
  int current_position = 0;
  string *tokens =
      (string *)malloc((unsigned long)buffer_size * sizeof(string));
  string token;
  int is_null;

  token = strtok(line, TOKEN_DELIMETERS);
  do {
    is_null = (token == NULL);
    tokens[current_position] = token;
    current_position += !is_null;

    int need_realloc = (current_position >= buffer_size);
    buffer_size += TOKEN_BUFFER_SIZE * need_realloc;
    string *clone = tokens;
    tokens =
        (string *)realloc((void *)clone, (size_t)buffer_size * sizeof(string));
    token = strtok(NULL, TOKEN_DELIMETERS);
  } while (!is_null);

  return tokens;
}

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
