#!/bin/bash
set -e
SRC="frontend/*.cpp retrieve/*.cpp upload/*.cpp parser/*.cpp auth/*.cpp *.cpp"

windres synd.rc -O coff -o synd_res.o
g++ -std=c++17 -O2 $SRC synd_res.o $(fltk-config --cxxflags) -o synd.exe \
    $(fltk-config --ldstaticflags) -lssh -lws2_32 -lfltk_images -lpng -lz -lfltk -lcomctl32 -lole32 -luuid -lws2_32 -lgdi32 -lfltk \
    -static-libgcc -static-libstdc++

rm -rf dist && mkdir dist
cp synd.exe dist/
ldd synd.exe | awk '/\/ucrt64\//{print $3}' | sort -u | while read dll; do cp "$dll" dist/; done
cp "/c/Program Files (x86)/cloudflared/cloudflared.exe" dist/

mkdir -p dist/frontend/icons
cp frontend/icons/*.png dist/frontend/icons/

ls dist
