sync
umount /dev/ram0
mount -t vfat /dev/ram0 /mnt/rd -o loop,umask=000 -v
