#!/bin/bash

echo "images/0013/"
for i in {1..6}
do
	pushd .
	echo -n "06 " 
	cd demo06
	./pipeline_driver.exe -d ../../cfg/images/0013/ -n 100 -m | grep '^frame time of [0-9]* ms ([0-9\.]*fps)'
	popd

	pushd .
	echo -n "07 " 
	cd demo07
	./pipeline_driver.exe -d ../../cfg/images/0013/ -n 100 | grep '^frame time of [0-9]* ms ([0-9\.]*fps)'
	popd
done

echo "images/0014/"
for i in {1..6}
do
	pushd .
	echo -n "06 " 
	cd demo06
	./pipeline_driver.exe -d ../../cfg/images/0014/ -n 100 -m | grep '^frame time of [0-9]* ms ([0-9\.]*fps)'
	popd

	pushd .
	echo -n "07 " 
	cd demo07
	./pipeline_driver.exe -d ../../cfg/images/0014/ -n 100 | grep '^frame time of [0-9]* ms ([0-9\.]*fps)'
	popd
done

echo "images/0015/"
for i in {1..6}
do
	pushd .
	echo -n "06 " 
	cd demo06
	./pipeline_driver.exe -d ../../cfg/images/0015/ -n 100 -m | grep '^frame time of [0-9]* ms ([0-9\.]*fps)'
	popd

	pushd .
	echo -n "07 " 
	cd demo07
	./pipeline_driver.exe -d ../../cfg/images/0015/ -n 100 | grep '^frame time of [0-9]* ms ([0-9\.]*fps)'
	popd
done
