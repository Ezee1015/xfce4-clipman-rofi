# xfce4-clipman-rofi

Open the [xfce4-clipman](https://docs.xfce.org/panel-plugins/xfce4-clipman-plugin/start) clipboard history with [rofi](https://github.com/davatorium/rofi).

## Installation
```bash
make
make install
```

## Usage
```bash
~/.local/share/rofi/xfce4-clipman-rofi
```

It will ask you with rofi which item you want to copy from the clipboard.

> For i3 users: `bindsym $mod+c exec ~/.local/share/rofi/xfce4-clipman-rofi`
