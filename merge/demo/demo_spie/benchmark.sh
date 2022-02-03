#!/bin/bash

echo "images/0013/"
for i in {1..6}
do
	echo -n "Dataflow " 
	./spie_driver.exe -d ../../cfg/images/0013/ -n 100 -s 0 -t 0 -m 0 | grep '^frame time of [0-9]* ms ([0-9\.]*fps)'
	echo -n "Serial "
	./spie_driver.exe -d ../../cfg/images/0013/ -n 100 -s 1 -t 0 -m 2 | grep '^frame time of [0-9]* ms ([0-9\.]*fps)'
done

echo "images/0014/"
for i in {1..6}
do
	echo -n "Dataflow " 
	./spie_driver.exe -d ../../cfg/images/0014/ -n 100 -s 0 -t 0 -m 2 | grep '^frame time of [0-9]* ms ([0-9\.]*fps)'
	echo -n "Serial "
	./spie_driver.exe -d ../../cfg/images/0014/ -n 100 -s 1 -t 0 -m 2 | grep '^frame time of [0-9]* ms ([0-9\.]*fps)'
done

echo "images/0015/"
for i in {1..6}
do
	echo -n "Dataflow " 
	./spie_driver.exe -d ../../cfg/images/0015/ -n 100 -s 0 -t 0 -m 2 | grep '^frame time of [0-9]* ms ([0-9\.]*fps)'
	echo -n "Serial "
	./spie_driver.exe -d ../../cfg/images/0015/ -n 100 -s 1 -t 0 -m 2 | grep '^frame time of [0-9]* ms ([0-9\.]*fps)'
done
