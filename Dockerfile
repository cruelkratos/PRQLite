FROM debian:bookworm-slim 

RUN apt-get update && apt-get install -y \
	g++ \
	cmake \ 
	ninja-build 

WORKDIR /app

COPY . .

RUN cmake -S . -B build -G Ninja
RUN cmake --build build

CMD ["./build/src/db"]