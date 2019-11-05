clean:
	rm -rf build

cmake:
	@mkdir -p build; cd build; cmake -G"Ninja" ..

build:
	@cd build; ninja

test: build
	@cd build; ninja test

run: build
	@cd build/src/app; ./Playground

docs:
	@doxygen

.PHONY: docs build
