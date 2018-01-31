#/bin/sh
./gencab EzShare.cab Flash.cab
split -d -b 128k Flash.cab


for f in `ls x??`; do
if [ -f ./$f ]; then
/usr/bin/mv ./$f $f.img
fi
done


