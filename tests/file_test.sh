#!/bin/bash

TIME=360

if [ $# -ne 2 ]; then
	echo "Usage: canal_tool_binary program_source";
	exit 2
fi

#Get filename and extension of input file
filename=$(basename "$2");
extension="${filename##*.}";
#Change extension to .s
s_filename="${2%%.c}.s"


#Run script
ulimit -t $TIME
$1 $2 -e "set no-missing" -e run -e quit
exit_status=$?

#Check exit status
if [ $exit_status -ne 0 ]; then
	exit $exit_status
fi

#Check for failed compilation
if [ $extension = "c" -a ! -f $s_filename ]; then
	echo "Compilation failed"
	exit 3
fi

#Delete .s file it was compiled and everything worked
if [ $extension = "c" ]; then
	rm $s_filename
fi
exit 0
