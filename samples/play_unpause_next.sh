#!/bin/sh
# Play current or goto next song, Alexandre David

if xmmsctrl running; then
 if xmmsctrl playing; then
  if xmmsctrl paused; then
   xmmsctrl pause;
  else
   xmmsctrl next;
  fi
 else
  xmmsctrl play
 fi
else
 xmmsctrl launch play
fi
