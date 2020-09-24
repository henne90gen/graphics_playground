all: cmake build test

clean:
	@rm -rf build

clean-release:
	@rm -rf build

cmake:
	@mkdir -p build; cd build; cmake -G"Ninja" ..

cmake-release:
	@mkdir -p build-release; cd build-release; cmake -G"Ninja" -DCMAKE_BUILD_TYPE=Release ..

build:
	@cd build; ninja

build-release:
	@cd build-release; ninja

test: build
	@cd build/bin; ./Tests

run: build
	@cd build/bin; ./Playground

run-release: build-release
	@cd build-release/bin; ./Playground

run-bench-all: build-release
	python -m scripts bench

run-bench-marching-cubes: build-release
	@cd build/bin; ./MarchingCubesBench

run-bench-meta-balls: build-release
	@cd build/bin; ./MetaBallsBench

run-bench-xyz-loader-count-lines: build-release
	@cd build/bin; ./XyzLoaderCountLinesBench

run-bench-xyz-loader-load: build-release
	@cd build/bin; ./XyzLoaderLoadBench

run-bench-quad-tree: build-release
	@cd build/bin; ./QuadTreeBench

run-bench-graph-vis: build-release
	@cd build/bin; ./GraphVisBench

clang-tidy:
	@mkdir -p build; cd build; cmake -G"Ninja" -DRUN_CLANG_TIDY=ON ..; ninja > clang-tidy-report.txt

analyze-clang-tidy:
	@. venv/bin/activate; python -m scripts analyze

docs:
	@doxygen

cpu-performance:
	sudo cpupower frequency-set --governor performance

cpu-powersave:
	sudo cpupower frequency-set --governor powersave

download-gis-data:
	@. venv/bin/activate; python -m scripts download-gis-data

.PHONY: docs build build-release
