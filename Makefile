HEADERS=$(wildcard ./src/*.h)
SOURCES=$(wildcard ./src/*.c)
OBJECTS=$(patsubst ./src/%.c,./bin/%.o,$(SOURCES))

./example/main.bin: ./example/main.hex
	@ xxd -r ./example/main.hex > ./example/main.bin

$(OBJECTS): $(SOURCES)
	$(foreach x,$(SOURCES),@ gcc -c -o $(patsubst ./src/%.c,./bin/%.o,$x) $x -Wall;)

main: $(OBJECTS)
	@ gcc -o main $(OBJECTS)

.PHONY: build
build: main

.PHONY: run
run: build ./example/main.bin
	@ ./main ./example/main.bin

.PHONY: clean
clean:
	@ rm main
	@ rm ./bin/*.o
