CC=gcc

todo:
	$(CC) -o out main.c init-funcs.c window.c todo.c ./string/string.c -lncurses

commit:
	git add .
	git commit -m "$(m)"
	git push

clean:
	rm out*