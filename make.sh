#!/bin/bash

make clean 
make all

var=$(lsof -i:61005 | sed -n '2p' | awk '{print $2}')
kill $var
