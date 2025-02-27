compile: read_clipman.c send_to_clipboard.c
	gcc read_clipman.c -o read_clipman -g
	gcc send_to_clipboard.c -o send_to_clipboard -g

install: read_clipman send_to_clipboard
	mkdir -p ~/.local/share/rofi
	mv read_clipman ~/.local/share/rofi/
	mv send_to_clipboard ~/.local/share/rofi/

clean:
	rm read_clipman send_to_clipboard
