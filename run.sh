#!/bin/zsh

for file in ./main_*.cpp; do
	echo "-------- Running file $file --------"
	g++ -I. -I$SYSTEMC_HOME/include -L$SYSTEMC_HOME/lib-linux64 -lsystemc $file cache.cpp && ./a.out > ./evaluation/${file:0:-4}.txt
	echo "\n"
	echo "\n"
done         
rm ./a.out                       
