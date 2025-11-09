CFILE := xfce4-clipman-rofi.c
EXEC := xfce4-clipman-rofi

all: $(EXEC)

$(EXEC): $(CFILE)
	gcc $(CFLAGS) $(CFILE) -o $(EXEC)

debug:
	$(MAKE) CFLAGS="-g" all

.PHONY = clean
clean:
	rm $(EXEC)

.PHONY = install
install: $(EXEC)
	mkdir -p ~/.local/share/rofi
	mv $(EXEC) ~/.local/share/rofi/

.PHONY = uninstall
uninstall:
	rm ~/.local/share/rofi/$(EXEC)
