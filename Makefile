libs = -lraylib -lsqlite3
warnings = -Wall -Wpedantic -Wextra -Werror

./out/umai: ./src/*.c ./src/*.h ./src/ui/*.c ./src/ui/*.h
	mkdir -p out
	gcc src/*.c src/ui/*.c -o ./out/umai $(libs) $(warnings) -g

.PHONY: run
run: ./out/umai
	./out/umai
