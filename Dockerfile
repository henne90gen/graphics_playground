FROM henne90gen/opengl:3.1

RUN mkdir /app
WORKDIR /app

RUN git config --global http.sslverify false
RUN apt-get update && apt-get dist-upgrade -y && apt-get install -y libssl-dev
COPY build.sh /build.sh
CMD ["/build.sh"]

RUN ln -s /usr/bin/python3 /usr/bin/python
