#!/bin/sh
# November 17th, 2001, Jean Delvare
# Requires xmmsctrl 1.6 or later

if xmmsctrl running
then
	if xmmsctrl playing
	then
		if xmmsctrl is_stream
		then
			vol=`xmmsctrl getvol`
			if [ $vol -eq 0 ]
			then
				vol=`cat $HOME/.xmms/vol`
				xmmsctrl vol $vol
			else
				echo $vol > $HOME/.xmms/vol
				xmmsctrl vol 0
			fi
		else
			xmmsctrl pause
		fi
	else
		xmmsctrl play
	fi
else
	xmmsctrl launch play
fi
