#!/bin/sh
# This little shell program tests if xmms playlist is displayed,
# Alexandre David

state=$(xprop -name "$1" | grep "window state:" | cut -d\  -f3 )

if [ "$state" = "Normal" ]; then
 exit 0
else
 exit 1
fi
