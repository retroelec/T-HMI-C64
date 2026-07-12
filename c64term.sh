#xterm -fa 'Courier' -fs 2 -geometry 320x200 -e ./c64term &
kitty -o font_family='Courier' -o font_size=4 -o remember_window_size=no -o initial_window_width=320c -o initial_window_height=200c -o modify_font="cell_height 50%" ./c64term &

