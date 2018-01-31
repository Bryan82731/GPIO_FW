#!/bin/bash

if [ -f .chip ];then
chip_version=`cat .chip`
if [ $chip_version -ge 10 ]; then
port=4281
else
port=4181
fi

if [ $chip_version -eq 10 ]; then
bootdata=bootrom_data.img
fi

checkfile=0

#for filename in newconf.img all.img irom.img boot.img
#for filename in newconf.img all.img bootrom_data.img
for filename in newconf.img all.img $bootdata language.img
do

if [ $chip_version -ge 10 ]; then
load_start=0x8A000000
else
load_start=0x80200000
fi
#load_start=0x89000000

case "$filename" in
'newconf.img')
if [ $chip_version -ge 10 ]; then
spi_start=0x8B020000
else
spi_start=0x81000000
fi
;;
'all.img')
if [ $chip_version -ge 10 ]; then
spi_start=0x8B030000
else
spi_start=0x81010000
fi
;;
'irom.img')
if [ $chip_version -ge 10 ]; then
spi_start=0x8B100000
else
continue
fi
;;
'boot.img')
if [ $chip_version -ge 10 ]; then
spi_start=0x8B010000
else
continue
fi
;;
'bootrom_data.img')
if [ $chip_version -eq 10 ]; then
spi_start=0x8B010000
else
continue
fi
;;
'language.img')
if [ $chip_version -eq 11 ]; then
spi_start=0x8B090000
echo "[RTK]spi_start = 0x8B090000"
else
continue
fi
;;

esac

size=`stat -c %s $filename`

if [ $checkfile -eq 1 ];then
sed 's/file flashprog_dbg.exe/file flashprog-'$port'_dbg.exe/g;s/set $file_size = .*/set $file_size = '$size'/g;s/  restore tmpfile/  restore '$filename'/g;s/dump binary memory tmpfile/dump binary memory '$filename'.chk/g;s/set $spi_addr = .*/set $spi_addr = '$spi_start'/g;s/set $load_addr = .*/set $load_addr = '$load_start'/g;s/target remote localhost:.*/target remote localhost:'$port'/g' gdb_flash.txt > gdb_flash.scr
else
sed 's/file flashprog_dbg.exe/file flashprog-'$port'_dbg.exe/g;s/set $file_size = .*/set $file_size = '$size'/g;s/  restore tmpfile/  restore '$filename'/g;s/dump binary memory tmpfile/#dump binary memory '$filename'.chk/g;s/set $spi_addr = .*/set $spi_addr = '$spi_start'/g;s/set $load_addr = .*/set $load_addr = '$load_start'/g;s/target remote localhost:.*/target remote localhost:'$port'/g' gdb_flash.txt > gdb_flash.scr
fi

rsdk-elf-gdb -x gdb_flash.scr

if [ $checkfile -eq 1 ];then
diff $filename $filename.chk

if [ $? != 0 ]; then
  echo $filename " comparison error"
  break
else
  echo $filename " checked successfully"
fi
fi
done

else
echo "Error! No chip configuration found!"
echo "Please run make in the upper directory first."
fi
