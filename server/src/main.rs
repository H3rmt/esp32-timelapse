use std::env;
use std::fs::File;
use std::io::Write;
use axum::Router;
use std::net::SocketAddr;
use chrono::offset::Local;
use axum::body::Bytes;
use axum::http::{HeaderMap, StatusCode};
use axum::routing::post;

#[tokio::main]
async fn main() {
    let app = Router::new()
        .route("/upload", post(bytes));

    let addr = SocketAddr::from(([192, 168, 187, 32], 8080));
    // let addr = SocketAddr::from(([127, 0, 0, 1], 8080));
    println!("listening on {}", addr);

    env::var("SECRET").expect("Error: SECRET not found");

    axum::Server::bind(&addr)
        .serve(app.into_make_service())
        .await
        .unwrap();
}

async fn bytes(body: Bytes, headers: HeaderMap) -> Result<(), StatusCode> {
    let time_str = Local::now().format("%d-%m-%Y_%H:%M:%S");

    let auth = headers.get("Authorisation").ok_or_else(|| {
        eprintln!("Error: Authorisation header not found");
        StatusCode::UNAUTHORIZED
    })?.to_str().map_err(|e| {
        eprintln!("Error: Authorisation header is not a string ({e})");
        StatusCode::BAD_REQUEST
    })?;

    let secret = env::var("SECRET").map_err(|e| {
        eprintln!("Error: SECRET not found ({e})");
        StatusCode::INTERNAL_SERVER_ERROR
    })?;

    if auth != secret {
        eprintln!("Error: Authorisation header is not correct ({auth})");
        return Err(StatusCode::UNAUTHORIZED);
    }

    let count = headers.get("Count").ok_or_else(|| {
        eprintln!("Error: Count header not found");
        StatusCode::BAD_REQUEST
    })?.to_str().map_err(|e| {
        eprintln!("Error: Count header is not a string ({e})");
        StatusCode::BAD_REQUEST
    })?.parse::<u32>().map_err(|e| {
        eprintln!("Error: Count header is not a number ({e})");
        StatusCode::BAD_REQUEST
    })?;

    let mut file = File::create(format!("imgs/{}.{}.jpg", time_str, count)).map_err(|e| {
        eprintln!("Error: Could not create file ({e})");
        StatusCode::INTERNAL_SERVER_ERROR
    })?;

    file.write_all(&body).map_err(|e| {
        eprintln!("Error: Could not write to file ({e})");
        StatusCode::INTERNAL_SERVER_ERROR
    })?;

    Ok(())
}
