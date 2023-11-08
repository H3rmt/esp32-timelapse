FROM rust:1.73.0 as build
WORKDIR /app

# copy over your manifests
COPY ./Cargo.lock ./Cargo.lock
COPY ./Cargo.toml ./Cargo.toml

# this build step will cache your dependencies
#RUN cargo build --release
#RUN rm src/*.

# copy your source tree
COPY ./src ./src

# build for release
#RUN rm ./target/release/deps/esp32-server*
RUN cargo build --release

CMD ["/app/target/release/esp-32-img-server"]