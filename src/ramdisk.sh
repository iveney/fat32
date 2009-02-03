#!/bin/bash

# option of mkfs.vfat
# F : FAT size (12,16,32)
# f : FAT number
# S : logical sector size,default is 512, be power of 2,greater than 512
# s : sector per cluster
# R : number of reserved sectors FAT32 default is 32, at least 2

sudo umount -v /mnt/rd
/sbin/mkfs.vfat -v -F 32 -f 2 -S 512 -s 1 -R 32 /dev/ram0
sudo mount -t vfat /dev/ram0 /mnt/rd -o loop,umask=000 -v
