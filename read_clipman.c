#include <stdio.h>
#include <stdbool.h>
#include <stdlib.h>
#include <string.h>

bool print_line(FILE *f) {
  char c;
  int move = 1;
  bool escaped = false;

  while ( (c = fgetc(f)) != ';' || escaped) {
    if (c == EOF) {
      fprintf(stderr, "File format error! EOF reached\n");
      return true;
    }

    switch (c) {
      case '\\':
        if (escaped) printf("\\\\");
        escaped = !escaped;
        break;

      case 's':
        putchar( (escaped) ? ' ' : c );
        escaped = false;
        break;

      case 't':
        putchar( (escaped) ? '\t' : c );
        escaped = false;
        break;

      case ';':
        putchar(';');
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
          return false;
        }
        putchar(c);
        escaped = false;
        break;
    }

    move++;
  }
  putchar('\n');
  fseek(f, -1 * move, SEEK_CUR);

  return true;
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

  // '-4': skip: last semicolon and 2 break line (one from xfce, other for
  //       POSIX) and position the cursor in the last char of the sentence. See also:
  //       https://stackoverflow.com/questions/729692/why-should-text-files-end-with-a-newline
  // '14': skip: "[texts]\ntexts="
  int pos;
  if ( fseek(f, -4, SEEK_END) || (pos = ftell(f)) < 14 ) {
    fprintf(stderr, "File format error! File: %s\n", path);
    free(path);
    return 1;
  }
  free(path);

  char c;
  while (pos > 14) {
    if ( (c = fgetc(f)) == ';' ) {
      int x = 1;
      bool escaped = false;
      while ( !fseek(f, -2, SEEK_CUR) && (c = fgetc(f)) == '\\' ) {
        escaped = !escaped;
        x++;
      }
      fseek(f, x, SEEK_CUR);

      if (!escaped && !print_line(f)) {
        fclose(f);
        return 1;
      }
    }

    fseek(f, -2, SEEK_CUR);
    pos--;
  }
  print_line(f);

  fclose(f);
  return 0;
}
