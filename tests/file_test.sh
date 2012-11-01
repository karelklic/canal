#!/bin/bash

TIME=20

if [ $# -ne 2 ]; then
	echo "Usage: canal_tool_binary program_source";
	exit 2
fi

ulimit -t $TIME
$1 $2 -e run -e quit
exit $?
