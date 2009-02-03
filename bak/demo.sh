#!/bin/bash
dd if=/dev/zero of=filesystem bs=1M count=64
/sbin/mkfs.vfat -v -F 32 -f 2 -S 512 -s 1 -R 32 ./filesystem
mount -t vfat ./filesystem ./rd -o loop,umask=000 -v

echo 123 > ./rd/A.TXT
# file ``ascii'' contains some ascii characters
cat ascii > ./rd/B.TXT
dd if=/dev/urandom of=./rd/C.TXT bs=count=1
# file ``raw'' contains some raw contents
cat raw > ./rd/D.TXT

sync
