main: src/main.c
	mkdir -p build
	gcc -o build/main src/main.c -lcjson -lcurl