#!/bin/bash

checkfile=0

#for filename in newconf.img all.img irom.img boot.img
#for filename in newconf.img all.img bootrom_data.img
for filename in newconf.img all.img 
do

load_start=0x8A000000
#load_start=0x89000000

case "$filename" in
'newconf.img')
spi_start=0x8B020000
;;
'all.img')
spi_start=0x8B030000
;;
'irom.img')
spi_start=0x8B100000
;;
'boot.img')
spi_start=0x8B010000
;;
'bootrom_data.img')
spi_start=0x8B010000
;;

esac

size=`stat -c %s $filename`

if [ $checkfile -eq 1 ];then
sed 's/set $file_size = .*/set $file_size = '$size'/g;s/  restore tmpfile/  restore '$filename'/g;s/dump binary memory tmpfile/dump binary memory '$filename'.chk/g;s/set $spi_addr = .*/set $spi_addr = '$spi_start'/g;s/set $load_addr = .*/set $load_addr = '$load_start'/g' gdb_flash.txt > gdb_flash.scr
else
sed 's/set $file_size = .*/set $file_size = '$size'/g;s/  restore tmpfile/  restore '$filename'/g;s/dump binary memory tmpfile/#dump binary memory '$filename'.chk/g;s/set $spi_addr = .*/set $spi_addr = '$spi_start'/g;s/set $load_addr = .*/set $load_addr = '$load_start'/g' gdb_flash.txt > gdb_flash.scr
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
