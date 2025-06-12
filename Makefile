CFILE := xfce4-clipman-rofi.c
EXEC := xfce4-clipman-rofi

all: $(EXEC)

$(EXEC): $(CFILE)
	gcc $(CFLAGS) $(CFILE) -o $(EXEC)

debug:
	$(MAKE) CFLAGS="-g" all

clean:
	rm $(EXEC)

install: $(EXEC)
	mkdir -p ~/.local/share/rofi
	mv $(EXEC) ~/.local/share/rofi/

uninstall:
	rm ~/.local/share/rofi/$(EXEC)
