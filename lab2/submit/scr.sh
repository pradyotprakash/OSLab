for i in 1
do
	echo $i
	echo $i >> out
	time ./multi-client 10.0.0.7 5001 $i 120 0 random >> out
done