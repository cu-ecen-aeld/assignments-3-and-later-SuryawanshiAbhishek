#!/bin/bash

#AESD Assignment-1 (Part-2)
#writer.sh
#Author: Abhishek Suryawanshi
#Reference: 
# 1.https://linuxhint.com/bash_mkdir_not_existent_path/  
# 2.https://stackoverflow.com/questions/6121091/how-to-extract-directory-path-from-file-path


#Declare variables to store command line argument

#first argument is a full path to a file (including filename) on the filesystem

writefile=$1

#second argument is a text string which will be written within this file

writestr=$2

#check whether number of argument is 2 or not

if [ $# -ne 2 ]
then
	echo "Invalid Number of argument"
	echo "Total Number of argument required: 2"
	echo "first argument path including filename to be written"
	echo "second argument is string to be written"
exit 1
fi

#check if directory exist or not if it doesn't then create new directory and file  

#extract directory from given path

DIR=${writefile%/*}

if [ !-d "$DIR" ]
then
	#create the directory
	mkdir -p "$DIR*"
    #create the file in the newly created directory
	touch $writefile
fi

#copy the input string in newly created file
echo $writestr > $writefile

#check if file successfully created or not

if [ !-f "$writefile" ]
then
	echo "File could not be created"
 
exit 1

else

  #if file is created then check the content are properly updated 
  count=$(grep "$writestr" "$writefile" | wc -l)

  #check if count matches with input string 

  if [ $count -ne  1 ]
  then

  	echo "created file is not updated  correctly"
  
  exit 1

fi

  echo "Input string '$writestr' is successfully added in the following path '$writefile'"
  
  exit 0
   
fi  


#end