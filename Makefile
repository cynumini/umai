./out/umai: ./src/*.c ./src/*.h
	mkdir -p out
	gcc src/*.c -o ./out/umai -lraylib -Wall -Wpedantic -Wextra -Werror -g

.PHONY: run
run: ./out/umai
	./out/umai
