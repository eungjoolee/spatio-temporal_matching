echo "images/0013/"
for i in {1..6}
do
	./combined_graph_driver.exe -n 100 -d ../../cfg/images/0013/ -s 2 | grep '^frame time of [0-9]* ms ([0-9\.]*fps)'
done

echo "images/0014/"
for i in {1..6}
do
	./combined_graph_driver.exe -n 100 -d ../../cfg/images/0014/ -s 2 | grep '^frame time of [0-9]* ms ([0-9\.]*fps)'
done

echo "images/0015/"
for i in {1..6}
do
        ./combined_graph_driver.exe -n 100 -d ../../cfg/images/0015/ -s 2 | grep '^frame time of [0-9]* ms ([0-9\.]*fps)'
done
