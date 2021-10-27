for f in *-symbolic.svg
do
	gtk-encode-symbolic-svg -o 32x32/ $f 32x32
	gtk-encode-symbolic-svg -o 48x48/ $f 48x48
	gtk-encode-symbolic-svg -o 64x64/ $f 64x64
done
