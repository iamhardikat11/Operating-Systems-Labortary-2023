CC=g++
CFLAGS=-DLOGS

all: libmemlab.a demo1 demo2 demo3 demo4 demo5

libmemlab.a: memlab.o
	ar -rcs libmemlab.a memlab.o

memlab.o: memlab.cpp memlab.h
	$(CC) $(CFLAGS) -c memlab.cpp -lpthread

demo1: demo1.o libmemlab.a
	$(CC) $(CFLAGS) -o demo1 demo1.o -L. -lmemlab -lpthread

demo1.o: demo1.cpp
	$(CC) $(CFLAGS) -c demo1.cpp

demo2: demo2.o libmemlab.a
	$(CC) $(CFLAGS) -o demo2 demo2.o -L. -lmemlab -lpthread

demo2.o: demo2.cpp
	$(CC) $(CFLAGS) -c demo2.cpp

demo3: demo3.o libmemlab.a
	$(CC) $(CFLAGS) -o demo3 demo3.o -L. -lmemlab -lpthread

demo3.o: demo3.cpp
	$(CC) $(CFLAGS) -c demo3.cpp

demo4: demo4.o libmemlab.a
	$(CC) $(CFLAGS) -o demo4 demo4.o -L. -lmemlab -lpthread

demo4.o: demo4.cpp
	$(CC) $(CFLAGS) -c demo4.cpp

demo5: demo5.o libmemlab.a
	$(CC) $(CFLAGS) -o demo5 demo5.o -L. -lmemlab -lpthread

demo5.o: demo5.cpp
	$(CC) $(CFLAGS) -c demo5.cpp

clean:
	rm -f libmemlab.a memlab.o demo1 demo1.o demo2 demo2.o demo3 demo3.o demo4 demo4.o demo5 demo5.o