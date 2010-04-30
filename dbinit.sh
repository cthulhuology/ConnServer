#!/bin/bash

export LD_LIBRARY_PATH:`pwd`
export CSID:0
export CSDBCONN:"dbname:devel user:dave"

psql devel < database.sql

./dbtool 0 add cs "name:dave.dloh.org,ipaddr:0.0.0.0,port:9996,nat_port:9996,admins:"
./dbtool 0 add cs "name:gene.dloh.org,ipaddr:0.0.0.0,port:9997,nat_port:9997,admins:"
./dbtool 0 add cs "name:aaron.dloh.org,ipaddr:0.0.0.0,port:9998,nat_port:9998,admins:"
./dbtool 0 add cs "name:anton.dloh.org,ipaddr:0.0.0.0,port:9999,nat_port:9999,admins:"

./dbtool 281474976710656 281474976710656 cache
#./dbtool 562949953421312 562949953421312 cache
#./dbtool 844424930131968 844424930131968 cache
#./dbtool 1125899906842624 1125899906842624 cache
#./dbtool 1407374883553280 1407374883553280 cache
#./dbtool 1688849860263936 1688849860263936 cache

