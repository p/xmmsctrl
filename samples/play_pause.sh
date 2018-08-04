#!/bin/sh
# Simple switch between pause and play, Alexandre David

if xmmsctrl running; then
 if xmmsctrl playing; then
  xmmsctrl pause
 else
  xmmsctrl play
 fi
else
 xmmsctrl launch play
fi
