clean:
	@rm -rf build

cmake:
	@mkdir -p build; cd build; cmake -G"Ninja" ..

build:
	@cd build; ninja

test: build
	@cd build; ninja test

run: build
	@cd build/src/app; ./Playground

clang-tidy: clean
	@mkdir -p build; cd build; cmake -G"Ninja" -DRUN_CLANG_TIDY=ON ..; ninja > clang-tidy-report.txt

docs:
	@doxygen

.PHONY: docs build
