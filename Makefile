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

clang-tidy:
	@mkdir -p build; cd build; cmake -G"Ninja" -DRUN_CLANG_TIDY=ON ..; ninja > clang-tidy-report.txt

analyze-clang-tidy:
	@. venv/bin/activate; python -m scripts analyze

docs:
	@doxygen

cmake-bench:
	@mkdir -p build; cd build; cmake -G"Ninja" -DCMAKE_BUILD_TYPE=Release ..

run-bench-marchin-cubes: build
	@cd build/src/logic; ./MarchingCubesBench --benchmark_repetitions=5

run-bench-meta-balls: build
	@cd build/src/logic; ./MetaBallsBench

.PHONY: docs build
