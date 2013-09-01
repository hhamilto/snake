all:            snake.c
		gcc -std=gnu99 -o snake snake.c
		
clean:
		rm -f snake
