#!/bin/bash

export LD_LIBRARY_PATH=`pwd`
export CSID=0
export CSDBCONN="dbname=devel user=dave"

MIN=0 MAX=281474976710656 ./database.sql | psql devel
