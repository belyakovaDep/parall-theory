if you need 'float' =>
	cmake -DUSE_FLOAT=ON .
	make
	./a
	
'double' =>
	cmake .
	make
	./a

Output with float:
	Sum is 8.19223
	
Output with double:
	Sum is 8.07976
