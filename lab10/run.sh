#!/bin/bash
echo 3 > /proc/sys/vm/drop_caches;
./a.out true read >> output;
notify-send "Done true read";
echo 3 > /proc/sys/vm/drop_caches;
./a.out false read >> output;
notify-send "Done false read";
echo 3 > /proc/sys/vm/drop_caches;
./a.out true write >> output;
notify-send "Done true write";
echo 3 > /proc/sys/vm/drop_caches;
./a.out false write >> output;
notify-send "Done false write";