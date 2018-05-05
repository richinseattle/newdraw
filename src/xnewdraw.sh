DOSEMU_FONTS=/usr/share/dosemu/Xfonts/

if [ -d $DOSEMU_FONTS ]
then
	xset +fp $DOSEMU_FONTS
else
	echo "Warning: dosemu fonts not found in $DOSEMU_FONTS"
fi

NEWDRAW_EXE=newdraw

if [ -f $NEWDRAW_EXE ]
then
	xterm -fn vga -e ./$NEWDRAW_EXE $*
else
	echo "Error: $NEWDRAW_EXE missing!"
fi
