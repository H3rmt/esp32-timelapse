FROM docker.io/rust:1.91.0-slim AS build
WORKDIR /app
COPY Cargo.toml Cargo.lock /app/
COPY src /app/src
RUN cargo build --release --locked

FROM debian:13.1-slim
RUN apt-get update && apt-get install -y ffmpeg && rm -rf /var/lib/apt/lists/*
COPY --from=build /app/target/release/esp-32-img-server /server
CMD ["/server"]