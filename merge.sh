#/bin/sh
rm EzShare.bin
rm Flash.cab
cd ezflash
./go.sh
cp Flash.cab ../
cd ..
cat 68DPSPIB.bin Flash.cab > EzShare.bin
