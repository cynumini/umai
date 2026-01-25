libs = -lraylib -lsqlite3
warnings = -Wall -Wpedantic -Wextra -Werror
flags = -Isakana/include $(warnings) $(libs) -g
libsakana = ./sakana/out/libsakana.a

src = ./src/main.c ./src/ui.c
headers = ./src/ui.h

$(libsakana): ./sakana/src/*.c ./sakana/include/SKN/*.h
	$(MAKE) -C ./sakana/

./out/umai: $(src) $(headers) $(libsakana)
	mkdir -p ./out
	gcc $(src) -o $@ $(flags) -L./sakana/out -lsakana

.PHONY: run
run: ./out/umai
	$^
