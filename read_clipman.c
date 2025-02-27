#include <stdio.h>
#include <stdbool.h>
#include <stdlib.h>
#include <string.h>

int main(void) {
  const char *home = getenv("HOME");
  if (!home) {
    fprintf(stderr, "Unable to get the HOME path");
    return 1;
  }
  const char *file = "/.cache/xfce4/clipman/textsrc"; // From home

  char *path = malloc(strlen(home) + strlen(file) + 1);
  strcpy(path, home);
  strcat(path, file);

  FILE *f = fopen(path, "r");
  if (!f) {
    fprintf(stderr, "Unable to open '%s'", path);
    free(path);
    return 1;
  }
  free(path);

  fseek(f, 14, SEEK_SET); // skip: "[texts]\ntexts="
  // I have to set a variable "new_element" because xfce4-clipman puts
  // ';' at the end.
  bool escaped = false, new_element = false;
  char c;
  while ( (c = fgetc(f)) != EOF && c != '\n') {
    if (new_element) {
      putchar('\n');
      new_element = false;
    }

    switch (c) {
      case '\\':
        if (escaped) printf("\\\\");
        escaped = !escaped;
        break;

      case ';':
        if (escaped) putchar(c);
        else new_element = true;
        escaped = false;
        break;

      case 's':
        putchar( (escaped) ? ' ' : c );
        escaped = false;
        break;

      case 't':
        putchar( (escaped) ? '\t' : c );
        escaped = false;
        break;

      case 'n':
        // Don't replace '\n' with a new line because rofi uses new line as a
        // separator. That's why I use `send_to_clipboard.c`. So '\n' should stay
        // as it is
        if (escaped) putchar('\\');
        putchar('n');
        escaped = false;
        break;

      default:
        if (escaped) {
          fprintf(stderr, "Error parsing the clipboard!. Unknown escape sequence: \\%c\n", c);
          fclose(f);
          return 1;
        }
        putchar(c);
        escaped = false;
        break;
    }
  }

  fclose(f);
  return 0;
}
