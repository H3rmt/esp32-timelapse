use std::env;
use std::fs::File;
use std::io::{Write};
use axum::Router;
use std::net::SocketAddr;
use axum::body::Bytes;
use chrono::offset::Local;
use axum::http::{HeaderMap, StatusCode};
use axum::response::IntoResponse;
use axum::routing::post;
use rand::Rng;

#[tokio::main]
async fn main() {
    env::var("SECRET").expect("Error: SECRET not found");

    let app = Router::new()
        .route("/upload", post(upload))
        .route("/finish", post(finish))
        .route("/start", post(start));

    let addr = SocketAddr::from(([0, 0, 0, 0], 8080));

    println!("listening on {}", addr);

    axum::Server::bind(&addr)
        .serve(app.into_make_service())
        .await
        .expect("server failed to start");
}

async fn finish(headers: HeaderMap) -> impl IntoResponse {
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

    let identifier = headers.get("Identifier").ok_or_else(|| {
        eprintln!("Error: Identifier header not found");
        StatusCode::BAD_REQUEST
    })?.to_str().map_err(|e| {
        eprintln!("Error: Identifier header is not a string ({e})");
        StatusCode::BAD_REQUEST
    })?;

    std::fs::rename(format!("imgs/{identifier}"), format!("imgs/{identifier}-{count}")).map_err(|e| {
        eprintln!("Error: Could not rename folder ({e})");
        StatusCode::INTERNAL_SERVER_ERROR
    })?;

    println!("Finished Folder {identifier}-{count}");
    Ok(())
}

async fn start(headers: HeaderMap) -> Result<String, StatusCode> {
    let time_str = Local::now().format("%y-%m-%d");
    let mut rng = rand::thread_rng();
    let identifier = format!("{time_str}_{}", rng.gen_range(100..999));

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

    std::fs::create_dir(format!("imgs/{identifier}")).map_err(|e| {
        eprintln!("Error: Could not create folder ({e})");
        StatusCode::INTERNAL_SERVER_ERROR
    })?;

    println!("Created new folder: {identifier}");
    Ok(identifier)
}

async fn upload(headers: HeaderMap, body: Bytes) -> impl IntoResponse {
    let time_str = Local::now().format("%y-%m-%d_%H:%M:%S");

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

    let identifier = headers.get("Identifier").ok_or_else(|| {
        eprintln!("Error: Identifier header not found");
        StatusCode::BAD_REQUEST
    })?.to_str().map_err(|e| {
        eprintln!("Error: Identifier header is not a string ({e})");
        StatusCode::BAD_REQUEST
    })?;

    let mut file = File::create(format!("imgs/{identifier}/{count}_{time_str}.jpg")).map_err(|e| {
        eprintln!("Error: Could not create file ({e})");
        StatusCode::INTERNAL_SERVER_ERROR
    })?;

    file.write_all(&body).map_err(|e| {
        eprintln!("Error: Could not write to file ({e})");
        StatusCode::INTERNAL_SERVER_ERROR
    })?;

    println!("Saved image: {time_str}.{count}.jpg");
    Ok(())
}
