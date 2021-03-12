rm -rf opk
mkdir -p opk
cp funkey/retro8 opk
cp vs2017/retro8/api.lua opk
cp ../data/default.funkey-s.desktop opk
cp ../data/icon.png opk
mksquashfs opk retro8.opk -all-root -noappend -no-exports -no-xattrs -no-progress > /dev/null
# rm -rf opk
