#!/bin/bash

TIME=360

if [ $# -ne 2 ]; then
	echo "Usage: canal_tool_binary program_source";
	exit 2
fi

ulimit -t $TIME
$1 $2 --no-missing -e run -e quit
exit_status=$?
if [ $exit_status -ne 0 ]; then
	exit $exit_status
fi
filename="${2##*/}"
if [ ! -f $filename ]; then
	echo "Compilation failed"
	exit 3
fi
exit 0
