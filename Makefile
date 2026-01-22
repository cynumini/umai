libs = -lraylib -lsqlite3
warnings = -Wall -Wpedantic -Wextra -Werror

src = ./src/main.c ./src/ui.c
headers = ./src/ui.h

./out/umai: $(src) $(headers)
	mkdir -p ./out
	gcc $(src) -o ./out/umai $(libs) $(warnings) -g

.PHONY: run
run: ./out/umai
	./out/umai
