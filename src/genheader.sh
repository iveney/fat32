#!/bin/bash

for i in $@
do
	name="${i%\.c}"
	filename="$name".h
	DEF=`echo $name | tr [[:lower:]] [[:upper:]] `
	HEADER="__${DEF}_H__"
	#echo "i=$i name=$name filename=$filename DEF=$DEF HEADER=$HEADER"
	echo "" > $filename
	echo "#ifndef $HEADER" > "$filename".temp
	echo "#define $HEADER" >> "$filename".temp
	cproto $i >> "$filename".temp
	echo "#endif " >> "$filename".temp
	mv "$filename".temp $filename
done
