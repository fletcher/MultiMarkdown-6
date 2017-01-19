#!/bin/bash
cp ../tests/MMD6Tests/Markdown\ Syntax.text ../build/speed.txt

cd ../build;


echo "MMD 6 - 8"
cat speed.txt{,}{,}{,} > speeda.txt
cat speeda.txt > speedbig.txt
/usr/bin/env time -p ./multimarkdown -c speedbig.txt > /dev/null

echo "MMD 6 - 16"
cat speeda.txt >> speedbig.txt
cat speedbig.txt > speeda.txt
/usr/bin/env time -p ./multimarkdown -c speedbig.txt > /dev/null

echo "MMD 6 - 32"
cat speeda.txt >> speedbig.txt
cat speedbig.txt > speeda.txt
/usr/bin/env time -p ./multimarkdown -c speedbig.txt > /dev/null

echo "MMD 6 - 64"
cat speeda.txt >> speedbig.txt
cat speedbig.txt > speeda.txt
/usr/bin/env time -p ./multimarkdown -c speedbig.txt > /dev/null

echo "MMD 6 - 128"
cat speeda.txt >> speedbig.txt
cat speedbig.txt > speeda.txt
/usr/bin/env time -p ./multimarkdown -c speedbig.txt > /dev/null

echo "MMD 6 - 256"
cat speeda.txt >> speedbig.txt
cat speedbig.txt > speeda.txt
/usr/bin/env time -p ./multimarkdown -c speedbig.txt > /dev/null

echo "MMD 6 - 512"
cat speeda.txt >> speedbig.txt
cat speedbig.txt > speeda.txt
/usr/bin/env time -p ./multimarkdown -c speedbig.txt > /dev/null

echo "MMD 6 - 1024"
cat speeda.txt >> speedbig.txt
cat speedbig.txt > speeda.txt
/usr/bin/env time -p ./multimarkdown -c speedbig.txt > /dev/null

echo "MMD 6 - 2048"
cat speeda.txt >> speedbig.txt
cat speedbig.txt > speeda.txt
/usr/bin/env time -p ./multimarkdown -c speedbig.txt > /dev/null

echo "MMD 6 - 4096"
cat speeda.txt >> speedbig.txt
cat speedbig.txt > speeda.txt
/usr/bin/env time -p ./multimarkdown -c speedbig.txt > /dev/null

rm speed.txt
rm speedbig.txt
rm speeda.txt
