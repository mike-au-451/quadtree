OBJS = quadtree.o

quadtree_test: $(OBJS) quadtree_test.o
	cc -std=gnu99 -Wall -g -O0 -o quadtree_test $(OBJS) quadtree_test.o

%.o: %.c
	cc -std=gnu99 -Wall -g -O0 -c $<

.PHONY: clean

clean:
	rm -f *.o quadtree_test 
