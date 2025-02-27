#include <stdio.h>
#include <stdbool.h>
#include <stdlib.h>

int main(void) {
  FILE* xclip = popen("xclip -i -sel clip", "w");

  if (!xclip) {
    fprintf(stderr, "popen() failed!");
    return 1;
  }

  bool escaped = false;
  char c;
  while ( (c = getchar()) != EOF && c != 10 ) { // Finish when rofi prints a '\n'
    switch (c) {
      case '\\':
        if (escaped) fputc('\\', xclip);
        escaped = !escaped;
        break;

      case 'n':
        if (escaped) fputc('\n', xclip);
        else fputc('n', xclip);
        escaped = false;
        break;

      default:
        if (escaped) {
          fprintf(stderr, "Error parsing the clipboard!. Unknown escape sequence: \\%c\n", c);
          pclose(xclip);
          return 1;
        }
        fputc(c, xclip);
        escaped = false;
        break;
    }
  }

  return WEXITSTATUS(pclose(xclip));
}
