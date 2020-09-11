all: cmake-release build test run-bench-marching-cubes run-bench-meta-balls

clean:
	@rm -rf build

cmake:
	@mkdir -p build; cd build; cmake -G"Ninja" ..

build:
	@cd build; ninja

test: build
	@cd build; CTEST_OUTPUT_ON_FAILURE=ON ninja test

run: build
	@cd build/src/app; ./Playground

run-bench-all: build
	python -m scripts bench

run-bench-marching-cubes: build
	@cd build/src/bench; ./MarchingCubesBench

run-bench-meta-balls: build
	@cd build/src/bench; ./MetaBallsBench

run-bench-xyz-loader-count-lines: build
	@cd build/src/bench; ./XyzLoaderCountLinesBench

run-bench-xyz-loader-load: build
	@cd build/src/bench; ./XyzLoaderLoadBench

run-bench-quad-tree: build
	@cd build/src/bench; ./QuadTreeBench

run-bench-graph-vis: build
	@cd build/src/bench; ./GraphVisBench

clang-tidy:
	@mkdir -p build; cd build; cmake -G"Ninja" -DRUN_CLANG_TIDY=ON ..; ninja > clang-tidy-report.txt

analyze-clang-tidy:
	@. venv/bin/activate; python -m scripts analyze

docs:
	@doxygen

cmake-release:
	@mkdir -p build; cd build; cmake -G"Ninja" -DCMAKE_BUILD_TYPE=Release ..

cpu-performance:
	sudo cpupower frequency-set --governor performance

cpu-powersave:
	sudo cpupower frequency-set --governor powersave

download-gis-data:
	@. venv/bin/activate; python -m scripts download-gis-data

.PHONY: docs build
