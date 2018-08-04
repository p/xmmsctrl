#!/bin/sh
# October 5th, 2001, Jean Delvare

if xmmsctrl running
then
	xmmsctrl next play
else
	xmmsctrl launch play
fi
