#include <stdio.h>
#include <stdbool.h>
#include <stdlib.h>
#include <string.h>

#define STR_BLOCK_SIZE 256
#define TEMP_PATH "/tmp/xfce4-clipman-rofi.temp"
#define MENU_CMD "rofi -i -dmenu"
#define CLIPBOARD_MANAGER_CMD "xclip -i -sel clip"

typedef struct {
  char *str;
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
    cb->info.size = STR_BLOCK_SIZE * (new_length/STR_BLOCK_SIZE+1);
    if ( !(cb->info.str = realloc(cb->info.str, cb->info.size)) )
      return false;
  }

  if (!cb->info.length) strcpy(cb->info.str, s);
  else strcat(cb->info.str, s);
  cb->info.length = new_length;

  return true;
}

bool append_char(Clipboard *cb, char c) {
  char s[2] = {c, '\0'};

  return append_str(cb, s);
}

void free_string(String *text) {
  free(text->str);
  text->size = 0;
  text->length = 0;
}

void free_clipboard(Clipboard **cb) {
  while (*cb) {
    Clipboard *aux = *cb;
    *cb = aux->next;
    free_string(&(aux->info));
    free(aux);
  }
}

bool load_clipboard(Clipboard **cb) {
  if (!cb) return false;

  const char *home = getenv("HOME");
  if (!home) {
    fprintf(stderr, "Unable to get the HOME path\n");
    return false;
  }
  const char *file = "/.cache/xfce4/clipman/textsrc"; // Path from home

  char *path = malloc(strlen(home) + strlen(file) + 1);
  strcpy(path, home);
  strcat(path, file);

  FILE *f = fopen(path, "r");
  if (!f) {
    fprintf(stderr, "Unable to open '%s'\n", path);
    free(path);
    return false;
  }
  free(path);

  fseek(f, 14, SEEK_SET); // skip: "[texts]\ntexts="
  bool escaped = false, new_element = true;
  char c;

  while ( (c = fgetc(f)) != '\n' && c != EOF) {
    if (new_element) {
      Clipboard *aux = *cb;
      *cb = malloc(sizeof(Clipboard));
      (*cb)->next = aux;
      (*cb)->info.str = malloc(STR_BLOCK_SIZE);
      (*cb)->info.size = STR_BLOCK_SIZE;
      (*cb)->info.length = 0;

      new_element = false;
    }

    switch (c) {
      case '\\':
        if (escaped && !append_str(*cb, "\\\\")) {
          fprintf(stderr, "Error while appending string\n");
          fclose(f);
          return false;
        }
        escaped = !escaped;
        break;

      case ';':
        if (!escaped) {
          new_element = true;
          escaped = false;
          break;
        }

        if (!append_char(*cb, ';')) {
          fprintf(stderr, "Error while appending char\n");
          fclose(f);
          return false;
        }
        escaped = false;
        break;

      case 's':
        if (!append_char(*cb, (escaped) ? ' ' : 's')) {
          fprintf(stderr, "Error while appending char\n");
          fclose(f);
          return false;
        }
        escaped = false;
        break;

      case 't':
        if (!append_char(*cb, (escaped) ? '\t' : 't')) {
          fprintf(stderr, "Error while appending char\n");
          fclose(f);
          return false;
        }
        escaped = false;
        break;

      case 'n':
        // Don't replace '\n' with a new line because rofi uses new line as a
        // separator.
        if (!append_str(*cb, (escaped) ? "\\n" : "n")) {
          fprintf(stderr, "Error while appending string or char\n");
          fclose(f);
          return false;
        }
        escaped = false;
        break;

      default:
        if (escaped) {
          fprintf(stderr, "Error parsing the clipboard!. Unknown escape sequence: \\%c\n", c);
          fclose(f);
          return false;
        }
        if (!append_char(*cb, c)) {
          fprintf(stderr, "Error while appending char\n");
          fclose(f);
          return false;
        }
        escaped = false;
        break;
    }
  }

  fclose(f);
  return true;
}

bool send_to_menu(Clipboard *cb) {
  FILE *menu = popen(MENU_CMD " > " TEMP_PATH, "w");

  if (!menu) {
    fprintf(stderr, "popen() failed!\n");
    return false;
  }

  while (cb) {
    fprintf(menu, "%s\n", cb->info.str);
    cb = cb->next;
  }

  return (WEXITSTATUS(pclose(menu)) == 0);
}

bool send_to_clipboard(void) {
  FILE *clip = popen(CLIPBOARD_MANAGER_CMD, "w");

  if (!clip) {
    fprintf(stderr, "popen() failed!\n");
    remove(TEMP_PATH);
    return false;
  }

  FILE *output = fopen(TEMP_PATH, "r");
  if (!output) {
    remove(TEMP_PATH);
    return false;
  }

  bool escaped = false;
  char c;
  // Escape new line at the end of the file:
  // https://stackoverflow.com/questions/729692/why-should-text-files-end-with-a-newline
  while ( (c = fgetc(output)) != '\n' && c != EOF ) {
    switch (c) {
      case '\\':
        if (escaped) fputc('\\', clip);
        escaped = !escaped;
        break;

      case 'n':
        if (escaped) fputc('\n', clip);
        else fputc('n', clip);
        escaped = false;
        break;

      default:
        if (escaped) {
          fprintf(stderr, "Error parsing the clipboard!. Unknown escape sequence: \\%c\n", c);
          fclose(output);
          remove(TEMP_PATH);
          pclose(clip);
          return false;
        }
        fputc(c, clip);
        escaped = false;
        break;
    }
  }

  fclose(output);
  remove(TEMP_PATH);
  return (WEXITSTATUS(pclose(clip)) == 0);
}

int main(void) {
  Clipboard *cb = NULL;

  if (!load_clipboard(&cb)) {
    free_clipboard(&cb);
    return 1;
  }

  if (!send_to_menu(cb)) {
    free_clipboard(&cb);
    return 2;
  }

  if (!send_to_clipboard()) {
    free_clipboard(&cb);
    return 3;
  }

  free_clipboard(&cb);
  return 0;
}
