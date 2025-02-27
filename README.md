# xfce4-clipman-rofi
Open the xfce4-clipman clipboard history with rofi.

## Installation
```bash
make
make install
```

## Usage
```bash
~/.local/share/rofi/read_clipman | rofi -dmenu | ~/.local/share/rofi/send_to_clipboard
```

> For i3 users: `bindsym $mod+c exec ~/.local/share/rofi/read_clipman | rofi -dmenu | ~/.local/share/rofi/send_to_clipboard`
