#include <stdio.h>
#include <stdbool.h>
#include <stdlib.h>
#include <string.h>

#define BLOCK_SIZE 256

typedef struct {
  char *t;
  size_t length;
  size_t size;
} String;

struct Clipboard {
  String info;
  struct Clipboard *next;
};
typedef struct Clipboard Clipboard;

bool append_str(Clipboard *cb, char *s) {
  if (!cb || !s) return false;

  size_t new_length = cb->info.length+strlen(s);

  if (new_length+1 > cb->info.size) {
    cb->info.size = BLOCK_SIZE * (new_length/BLOCK_SIZE+1);
    if ( !(cb->info.t = realloc(cb->info.t, cb->info.size)) )
      return false;
  }

  if (!cb->info.length) strcpy(cb->info.t, s);
  else strcat(cb->info.t, s);
  cb->info.length = new_length;

  return true;
}

bool append_char(Clipboard *cb, char c) {
  char s[2] = {c, '\0'};

  return append_str(cb, s);
}

void free_text(String *text) {
  free(text->t);
  text->size = 0;
  text->length = 0;
}

void free_clipboard(Clipboard **cb) {
  while (*cb) {
    Clipboard *aux = *cb;
    *cb = aux->next;
    free_text(&(aux->info));
    free(aux);
  }
}

int main(void) {
  const char *home = getenv("HOME");
  if (!home) {
    fprintf(stderr, "Unable to get the HOME path\n");
    return 1;
  }
  const char *file = "/.cache/xfce4/clipman/textsrc"; // From home

  char *path = malloc(strlen(home) + strlen(file) + 1);
  strcpy(path, home);
  strcat(path, file);

  FILE *f = fopen(path, "r");
  if (!f) {
    fprintf(stderr, "Unable to open '%s'\n", path);
    free(path);
    return 1;
  }
  free(path);

  fseek(f, 14, SEEK_SET); // skip: "[texts]\ntexts="
  Clipboard *cb = NULL;
  bool escaped = false, new_element = true;
  char c;

  while ( (c = fgetc(f)) != '\n' && c != EOF) {
    if (new_element) {
      Clipboard *aux = cb;
      cb = malloc(sizeof(Clipboard));
      cb->next = aux;
      cb->info.t = malloc(BLOCK_SIZE);
      cb->info.size = BLOCK_SIZE;
      cb->info.length = 0;

      new_element = false;
    }

    switch (c) {
      case '\\':
        if (escaped && !append_str(cb, "\\\\")) {
          fprintf(stderr, "Error while appending string\n");
          free_clipboard(&cb);
          fclose(f);
          return 1;
        }
        escaped = !escaped;
        break;

      case ';':
        if (!escaped) {
          new_element = true;
          escaped = false;
          break;
        }

        if (!append_char(cb, ';')) {
          fprintf(stderr, "Error while appending char\n");
          free_clipboard(&cb);
          fclose(f);
          return 1;
        }
        escaped = false;
        break;

      case 's':
        if (!append_char(cb, (escaped) ? ' ' : 's')) {
          fprintf(stderr, "Error while appending char\n");
          free_clipboard(&cb);
          fclose(f);
          return 1;
        }
        escaped = false;
        break;

      case 't':
        if (!append_char(cb, (escaped) ? '\t' : 't')) {
          fprintf(stderr, "Error while appending char\n");
          free_clipboard(&cb);
          fclose(f);
          return 1;
        }
        escaped = false;
        break;

      case 'n':
        // Don't replace '\n' with a new line because rofi uses new line as a
        // separator. That's why I use `send_to_clipboard.c`. So '\n' should stay
        // as it is
        if (!append_str(cb, (escaped) ? "\\n" : "n")) {
          fprintf(stderr, "Error while appending string or char\n");
          free_clipboard(&cb);
          fclose(f);
          return 1;
        }
        escaped = false;
        break;

      default:
        if (escaped) {
          fprintf(stderr, "Error parsing the clipboard!. Unknown escape sequence: \\%c\n", c);
          free_clipboard(&cb);
          fclose(f);
          return 1;
        }
        if (!append_char(cb, c)) {
          fprintf(stderr, "Error while appending char\n");
          free_clipboard(&cb);
          fclose(f);
          return 1;
        }
        escaped = false;
        break;
    }
  }


  Clipboard *aux = cb;
  while (aux) {
    printf("%s\n", aux->info.t);
    aux = aux->next;
  }

  free_clipboard(&cb);
  fclose(f);
  return 0;
}
