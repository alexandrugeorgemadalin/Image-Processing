build:
	gcc -Wall -Wextra image_processing.c -o bmp

run:
	./bmp

clean:
	rm bmp
