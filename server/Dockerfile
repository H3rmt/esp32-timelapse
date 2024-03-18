FROM rust:1.73.0 as build
WORKDIR /app
COPY Cargo.toml Cargo.lock /app/
COPY src /app/src
RUN cargo install --path .
RUN cd /usr/local/cargo/bin && echo $(ls)

FROM debian:bookworm-slim
RUN apt-get update & apt-get install -y extra-runtime-dependencies ffmpeg & rm -rf /var/lib/apt/lists/*

COPY --from=build /usr/local/cargo/bin/esp-32-img-server /app/esp-32-img-server
CMD ["/app/esp-32-img-server"]