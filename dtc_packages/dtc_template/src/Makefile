myProgram = dtc_template 

# Huanle Zhang
# www.huanlezhang.com

main: main.o
	$(CC) $(LDFLAGS) main.o -o $(myProgram)

main.o: main.c
	$(CC) $(CFLAGS) -c main.c
clean:
	rm *.o $(myProgram)
