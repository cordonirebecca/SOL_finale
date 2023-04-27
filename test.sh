#!/bin/bash


#./farm -n 1 -d testdir -q 1 file1.dat file2.dat file3.dat file4.dat file5.dat file10.dat file12.dat file13.dat file14.dat file15.dat file16.dat file17.dat file18.dat file20.dat file100.dat file116.dat file117.dat
valgrind  --leak-check=full --track-origins=yes -s ./farm file1.dat file2.dat file3.dat file4.dat file5.dat file10.dat file12.dat file13.dat file14.dat file15.dat file16.dat file17.dat file18.dat file20.dat file100.dat file116.dat file117.dat -d testdir 

PID_SERVER=$!
sleep 1

pkill -SIGINT farm

wait $PID_SERVER
