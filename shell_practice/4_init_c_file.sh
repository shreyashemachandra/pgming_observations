#!/bin/bash
fileName="";
if [ $# -eq 0 ]; then
	echo "File Name Not given, Please provide the FileName"
	read fileName
else
	echo "File name is given"
	fileName=$1
fi

echo "File Name: $fileName"

case $fileName in
	*.c ) echo "C File";;
	*) echo "Not a C File"; fileName=$fileName.c;;
esac

echo $fileName
echo "!!!File Initiated!!!"
echo "#include<stdio.h>" >> $fileName
echo "" >> $fileName
echo "int main(int argc,char * args[]){" >> $fileName
echo "		printf(\"Hello World\n\");" >> $fileName
echo "		return 0;" >> $fileName
echo "}" >> $fileName
