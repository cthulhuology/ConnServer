#!/bin/bash

. ../.config
let VERSIONNUMBER=`cat module_version`
export VERSIONNUMBER
VERSION="$VERSIONNUMBER"
export VERSION

echo $VERSION

case "$1" in
	build)
		make modules
	;;
	install)
		sudo mkdir -p /usr/local/lib/ConnServer
		sudo mkdir -p /usr/local/lib/ConnServer/python
		sudo cp -u *$VERSION.so /usr/local/lib/ConnServer	
		let VERSIONNUMBER=$VERSIONNUMBER+1
		echo $VERSIONNUMBER > module_version
	;;
	*)
		echo "Usage: $0 [build|install]";;
esac

