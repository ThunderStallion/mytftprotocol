#!/bin/bash

var=$(lsof -i:61005 | sed -n '2p' | awk '{print $2}')
kill $var

./tftpserver &
./tftpclient -r demo.txt
