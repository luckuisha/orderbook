.PHONY: make test clean

all:
	mkdir -p build
	g++ -std=c++23 -O3 -Isrc \
	src/main.cpp \
	src/order.cpp \
	src/order_book.cpp \
	-o build/main

test:
	mkdir -p build
	g++ -std=c++23 -O0 -g -Iexternal -Isrc \
		tests/main_test.cpp \
		tests/order_book_test.cpp \
		src/order.cpp \
		src/order_book.cpp \
		-o build/test_main
	./build/test_main

clean:
	rm -rf build