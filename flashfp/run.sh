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

for filename in newconf.img all.img $bootdata language.img RtkUndiDxe.rom usbimage.bin
do
echo "======================" + "PG file is:" + "$filename" + "======================"

if [ $chip_version -ge 10 ]; then
#load_start=0x8A000000
load_start=0x80040000
else
load_start=0x80200000
fi
#load_start=0x89000000

case "$filename" in
'usbimage.bin')
if [ $chip_version -ge 15 ]; then
spi_start=0x82100000
else
spi_start=0x81080000
fi
;;
'RtkUndiDxe.rom')
if [ $chip_version -ge 15 ]; then
spi_start=0x82000000
#spi_start=0x8B030000
else
continue
fi
;;
'newconf.img')
if [ $chip_version -ge 10 ]; then
spi_start=0x82020000
#spi_start=0x8B020000
else
spi_start=0x81000000
fi
;;
'all.img')
if [ $chip_version -ge 10 ]; then
spi_start=0x82030000
#spi_start=0x8B030000
else
spi_start=0x81010000
fi
;;
'irom.img')
if [ $chip_version -ge 10 ]; then
spi_start=0x82100000
#spi_start=0x8B100000
else
continue
fi
;;
'boot.img')
if [ $chip_version -ge 10 ]; then
spi_start=0x82010000
#spi_start=0x8B010000
else
continue
fi
;;
'bootrom_data.img')
if [ $chip_version -eq 10 ]; then
spi_start=0x82010000
#spi_start=0x8B010000
else
continue
fi
;;
'language.img')
if [ $chip_version -ge 13 ]; then
spi_start=0x820F0000
#spi_start=0x8B090000
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

#../../bin/rsdk-elf-gdb -x gdb_flash.scr
/home/work/RSDK/rsdk-4.9.2/cygwin/newlib/bin/rsdk-elf-gdb -x gdb_flash.scr

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
