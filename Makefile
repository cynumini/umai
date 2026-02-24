libs = -lraylib -lsqlite3
warnings = -Wall -Wpedantic -Wextra -Werror
flags = -Isakana/include $(warnings) $(libs) -g -std=c++20
libsakana = ./sakana/out/libsakana.a

src = ./src/main.cpp ./src/ui.cpp
headers = ./src/ui.hpp

$(libsakana): ./sakana/src/*.c ./sakana/include/SKN/*.h
	$(MAKE) -C ./sakana/

./out/umai: $(src) $(headers) $(libsakana)
	mkdir -p ./out
	g++ $(src) -o $@ $(flags) -L./sakana/out -lsakana

.PHONY: run
run: ./out/umai
	$^
