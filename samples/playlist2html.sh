#!/bin/sh
#
# Derived work from xmms_playlist2html.c originally
# found in xmms-extra.
# This scripts uses xmmsctrl to generate an html page
# of the current playlist, highlighting the current
# played song.
#
# author: Alexandre David
# license: GPL

xmmsctrl donothing 2>/dev/null || { echo "xmmsctrl not found!" 1>&2; exit 1; }
xmmsctrl running || { echo "xmms not running!" 1>&2; exit 2; }
echo "<html><title>XMMS Radio - Playlist</title><body bgcolor=#000000 text=#ffffff>"
xmmsctrl print "<div align=center><font size=6>%T <small>%M</small></font><br>[ %P/%l ]</div>%n"
echo "<div align=center><font size=6><a href=/xmms.pls>Listen to XMMS Radio!</a>"
echo "<table border=0 align=center cellspacing=2 cellpadding=5><tr><td align=right><b>#</b></td><td><b>Title</b></td><td><b>Time</b></td></tr>"
xmmsctrl eq "666666" neq "333333" eq "<i>" eq "</i>" print "<tr><td bgcolor=#%(1) align=right>%*P</td><td bgcolor=#%(1)>%(2)%*T%(3)</td><td align=center bgcolor=#%(1)>%*M</td></tr>%n"
echo "</table></body></html>"
