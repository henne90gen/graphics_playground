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

clang-tidy:
	@mkdir -p build; cd build; cmake -G"Ninja" -DRUN_CLANG_TIDY=ON ..; ninja > clang-tidy-report.txt

analyze-clang-tidy:
	@. venv/bin/activate; python -m scripts analyze

run-benchmarks:
	@. venv/bin/activate; python -m scripts bench -s

docs:
	@doxygen

cpu-performance:
	sudo cpupower frequency-set --governor performance

cpu-powersave:
	sudo cpupower frequency-set --governor powersave

download-gis-data:
	@. venv/bin/activate; python -m scripts download-gis-data

.PHONY: docs build build-release cmake
