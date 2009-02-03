#!/bin/bash
dd if=/dev/zero of=filesystem bs=1M count=16
/sbin/mkfs.vfat -v -F 32 -f 2 -S 512 -s 1 -R 32 ./filesystem
mount -t vfat ./filesystem ./rd -o loop,umask=000 -v

echo "void main(){}" > ./rd/MAIN.C
# file ``ascii'' contains some ascii characters
touch ./rd/MAKEFILE
dd if=/dev/urandom of=./rd/C.TXT bs=100 count=1

sync
