rm -rf opk
mkdir -p opk
cp opendingux/retro8 opk
cp vs2017/retro8/api.lua opk
cp ../data/default.gcw0.desktop opk
cp ../data/pico8_font.png opk
cp ../data/icon.png opk
mksquashfs opk retro8.opk -all-root -noappend -no-exports -no-xattrs -no-progress > /dev/null
# rm -rf opk
