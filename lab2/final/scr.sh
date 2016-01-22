ipaddr="10.0.0.2"
portno="5000"
timer="120"

rm -f out1 out2 out3
./client $ipaddr $portno

touch out1
# experiment number 1
echo "Experiment 1 running!"
for i in 1 2 3 4 5 6 7 8 9 10
do
	echo $i
	echo $i >> out1
	time ./multi-client $ipaddr $portno $i $timer 0 random >> out1
	./client $ipaddr $portno
done

touch out2
# experiment number 2
echo "Experiment 2 running!"
for i in 1 2 3 4 5 10 15 20 25 30
do
	echo $i
	echo $i >> out2
	time ./multi-client $ipaddr $portno $i $timer 1 random >> out2
	./client $ipaddr $portno
done

touch out3
# experiment number 3
echo "Experiment 3 running!"
for i in 1 2 3 4 5 6 7 8 9 10
do
	echo $i
	echo $i >> out3
	time ./multi-client $ipaddr $portno $i $timer 0 fixed >> out3
	./client $ipaddr $portno
done