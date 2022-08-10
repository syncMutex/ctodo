CC=gcc

todo:
	$(CC) -o out main.c init-funcs.c window.c todo.c ./string/string.c -lncurses

debug_run:
	$(CC) -o out main.c init-funcs.c window.c todo.c ./string/string.c -lncurses -g
	gdb out core -q

commit:
ifeq ($(strip $(m)),)
	@(echo "please provide a commit message [m="\<message\>"]"; exit 1)
endif
	git add .
	git commit -m "$(m)"
	git push

testc:

clean:
	rm out*