FROM henne90gen/opengl:3.1

RUN mkdir /app
WORKDIR /app

RUN git config --global http.sslverify false
RUN echo "#!/usr/bin/env bash \n\
set -e \n\
mkdir -p build \n\
cd build || exit 0 \n\
cmake .. -G Ninja -D CMAKE_BUILD_TYPE=Release -D CMAKE_C_COMPILER=/usr/bin/clang -D CMAKE_CXX_COMPILER=/usr/bin/clang++ \n\
cmake --build . \n" > /build.sh

RUN chmod +x /build.sh
CMD ["/build.sh"]

RUN ln -s /usr/bin/python3 /usr/bin/python
