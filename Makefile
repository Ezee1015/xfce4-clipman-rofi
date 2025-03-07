compile: xfce4-clipman-rofi.c
	gcc xfce4-clipman-rofi.c -o xfce4-clipman-rofi -g

install: xfce4-clipman-rofi
	mkdir -p ~/.local/share/rofi
	mv xfce4-clipman-rofi ~/.local/share/rofi/

clean:
	rm xfce4-clipman-rofi
