CC=gcc

todo:
	$(CC) -o out main.c init-funcs.c window.c todo.c ./string/string.c -lncurses

clean:
	rm out*