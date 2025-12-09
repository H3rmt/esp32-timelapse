FROM docker.io/rust:1.91.0-slim AS build
WORKDIR /app
COPY Cargo.toml Cargo.lock /app/
COPY src /app/src
RUN cargo build --release --locked

FROM debian:13.1-slim
RUN apt-get update && apt-get install -y ffmpeg && rm -rf /var/lib/apt/lists/*
COPY --from=build /app/target/release/esp-32-img-server /server
COPY docker-entry.sh /docker-entry.sh
RUN groupadd -r app && useradd -r -g app -d /nonexistent -s /usr/sbin/nologin app
ENTRYPOINT ["/docker-entry.sh"]
CMD ["/server"]