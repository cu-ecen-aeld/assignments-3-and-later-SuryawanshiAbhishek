#!/bin/bash

#AESD Assignment-1 (Part-1)
#finder.sh
#Author: Abhishek Suryawanshi
#Reference: https://www.halolinux.us/commands-for-ubuntu/searching-for-text-with-grep.html


#Declare variables to store command line argument

#first argument is a path to a directory on the filesystem

filesdir=$1

#second argument is a text string which will be searched

searchstr=$2

#check whether number of argument is 2 or not

if [ $# -ne 2 ]
then
	echo "Invalid Number of argument"
	echo "Total Number of argument required: 2"
	echo "first argument path to directory"
	echo "second argument is string to be search"
exit 1
fi

#check whether directory mention in the first argument is exist or not

if [ !-d "$filesdir" ]
then
	echo "Directory does not exists"
	echo "check path and file name "
exit 1

else
	#count number of files in the directory 
	X=$(find "$filesdir" -type f | wc -l)
	#count number of lines matching with given strings
	Y=$(grep -r "$searchstr" "$filesdir"| wc -l)
    
    echo "The number of files are $X and the number of matching lines are $Y"

exit 0

fi

#end
