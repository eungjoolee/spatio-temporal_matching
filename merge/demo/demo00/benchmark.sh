echo "images/0013/"
for i in {1..6}
do
	./driver.exe ../../cfg/images/0013/ 2 100 | grep '^frame time of [0-9]* ms ([0-9\.]*fps)'
done

echo "images/0014/"
for i in {1..6}
do
	./driver.exe ../../cfg/images/0014/ 2 100 | grep '^frame time of [0-9]* ms ([0-9\.]*fps)'
done

echo "images/0015/"
for i in {1..6}
do
        ./driver.exe ../../cfg/images/0015/ 2 100 | grep '^frame time of [0-9]* ms ([0-9\.]*fps)'
done
