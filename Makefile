all:
	gcc copy.c -o copy
	gcc remove.c -o remove

clean:
	rm copy remove