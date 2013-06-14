EXECUTABLE="extfs_blockcopy"
main:
	mkdir -p bin
	cc src/main.c -lext2fs -o bin/$(EXECUTABLE) -D_FILE_OFFSET_BITS=64
clean:
	rm $(EXECUTABLE)
