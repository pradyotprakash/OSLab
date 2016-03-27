#!/bin/bash

# part b
echo "" > output
rm -f a.out
gcc -w code.c
sudo echo 3 > /proc/sys/vm/drop_caches;
./a.out true read 2 >> output;
notify-send "Done true read";
echo "Exp1 Done"
sudo echo 3 > /proc/sys/vm/drop_caches;
./a.out false read 2 >> output;
notify-send "Done false read";
echo "Exp2 Done"
sudo echo 3 > /proc/sys/vm/drop_caches;
./a.out true write 2 >> output;
notify-send "Done true write";
echo "Exp3 Done"
sudo echo 3 > /proc/sys/vm/drop_caches;
./a.out false write 2 >> output;
notify-send "Done false write";
echo "Exp4 Done"

sudo echo 3 > /proc/sys/vm/drop_caches;
echo "Last experiment running"
./a.out false write 3 >> output;
echo "Done"
rm a.out