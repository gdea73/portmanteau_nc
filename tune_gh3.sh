# z axis will be average score over 1,000 games
# fix COL_SCORE_LO at -100, since only the constants' ratios are significant
xlbl="COL_SCORE_HI"
xmin=20
xval=$xmin
xmax=220
xstep=20
ylbl="COL_SCORE_SHORT"
ymin=120
ymax=150
ystep=10
games=1000
xsteps=$(((xmax-xmin)/xstep))
ysteps=$(((ymax-ymin)/ystep))
printf "total steps: $((xsteps*ysteps)) ($xsteps * $ysteps)\n"
while [ $xval -lt $xmax ]; do
	yval=$ymin
	xnext=$((xval+xstep))
	while [ $yval -lt $ymax ]; do
		printf "($xval, $yval):\n"
		ynext=$((yval+ystep))
		sed -i "s/$xlbl $xval/$xlbl $xnext/" ai/ai.h
		sed -i "s/$ylbl $yval/$ylbl $ynext/" ai/ai.h
		make -B ai &>/dev/null
		./portmanteau_ai $games gh3
		yval=$ynext
	done
	xval=$xnext
done
printf "done."
