HEADERS=$(wildcard ./src/*.h)
SOURCES=$(wildcard ./src/*.c)
OBJECTS=$(patsubst ./src/%.c,./bin/%.o,$(SOURCES))

./example/main.bin: ./example/main.hex
	@ xxd -r -c 8 ./example/main.hex > ./example/main.bin

$(OBJECTS): $(SOURCES)
	$(foreach x,$(SOURCES),@ gcc -O3 -c -o $(patsubst ./src/%.c,./bin/%.o,$x) $x -Wall -Wextra -Wno-strict-aliasing;)

caivm: $(OBJECTS)
	@ gcc -o caivm $(OBJECTS)

.PHONY: build
build: caivm

.PHONY: run
run: build ./example/main.bin
	@ ./caivm ./example/main.bin

.PHONY: clean
clean:
	@ rm caivm
	@ rm ./bin/*.o
