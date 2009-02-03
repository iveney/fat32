bak.fs is a my current filesystem img
with two files in it: a.txt & b.txt

we want to test that is the behaviour of our program
the same as that of the standart Fat32 driver in linux.

which means:

0. find if the path is correct( here, only parent dir need to be checked )
then check if the corresponding file exists
if found, goto 1,else 2


1. if a file already exist:
(1)modify its DirEntry, 
change  a.file starting cluster(w and a mode are different)
	b.file size
	c.time stamp(optional)

----------------in w mode----------------------------
(2)erase the previous FAT chain in **ALL** FAT table
----------------in w mode----------------------------

----------------in a mode----------------------------
(2)erase the previous FAT chain in **ALL** FAT table
----------------in a mode----------------------------

(3)allocate new FAT chain to write into.
notice: write file one after another cluster( because there may be not enough space,
power failure, etc.)
also, during the write process, we should update the DirEntry and FAT table accordingly to keep track
of the current state

2. if a file does not exist
allocate a new DirEntry for it,and do as 1

about allocate entry:
first we locate the containing dir of the file.
once found, we check if we can find:

if we can not find:
search for an available dir entry in the current directory's cluster chain
if not, allocate a new cluster , update FAT,
 and allocate a new entry(find available entry again)


