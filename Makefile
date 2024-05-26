CC=gcc --std=gnu99 -g
EXE=minibf

main: minibf
	$(CC) minibf.c -o $(EXE)

clean: 
	rm $(EXE)
