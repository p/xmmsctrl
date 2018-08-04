#!/bin/sh
# November 20th, 2001, Jean Delvare

if xmmsctrl running
then
	xmmsctrl prev play
else
	xmmsctrl launch play
fi
