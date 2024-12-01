
for f in *-symbolic.svg
do
	gtk4-encode-symbolic-svg -o hicolor/32x32/actions $f 32x32
	gtk4-encode-symbolic-svg -o hicolor/48x48/actions $f 48x48
	gtk4-encode-symbolic-svg -o hicolor/64x64/actions $f 64x64
done
