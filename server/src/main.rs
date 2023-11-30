mod export;

use std::env;
use std::net::SocketAddr;
use axum::{routing::get, routing::post, Router, debug_handler};
use axum::body::Bytes;
use axum::extract::{Query};
use chrono::offset::Local;
use axum::http::{StatusCode};
use axum::response::IntoResponse;
use axum_auth::AuthBearer;
use rand::Rng;
use serde::Deserialize;
use tokio::fs::File;
use tokio::io::AsyncWriteExt;

#[tokio::main]
async fn main() {
    env::var("SECRET").expect("Error: SECRET not found");
    env::var("FPS").expect("Error: FPS not found"); 

    let app = Router::new()
        .route("/upload", post(upload))
        .route("/finish", get(finish))
        .route("/start", get(start));

    let addr = SocketAddr::from(([0, 0, 0, 0], 8080));

    println!("listening on {}", addr);

    axum::Server::bind(&addr)
        .serve(app.into_make_service())
        .await
        .expect("server failed to start");
}

#[derive(Debug, Deserialize)]
#[allow(dead_code)]
struct Params {
    count: Option<i32>,
    identifier: Option<String>,
}

// #[debug_handler]
async fn finish(AuthBearer(token): AuthBearer, Query(params): Query<Params>) -> impl IntoResponse {
    let secret = env::var("SECRET").map_err(|e| {
        eprintln!("Error: SECRET not found ({e})");
        StatusCode::INTERNAL_SERVER_ERROR
    })?;

    if token != secret {
        eprintln!("Error: Authorisation is not correct ({token})");
        return Err(StatusCode::UNAUTHORIZED);
    }

    let count = params.count.ok_or_else(|| {
        eprintln!("Error: Count param not found");
        StatusCode::BAD_REQUEST
    })?;

    let identifier = params.identifier.ok_or_else(|| {
        eprintln!("Error: Identifier param not found");
        StatusCode::BAD_REQUEST
    })?;

    std::fs::rename(format!("imgs/{identifier}"), format!("imgs/{identifier}-{count}")).map_err(|e| {
    	eprintln!("Error: Could not rename folder ({e})");
    	StatusCode::INTERNAL_SERVER_ERROR
    })?;

    println!("Finished Folder {identifier}-{count}");

    let fps = env::var("FPS").map_err(|e| {
        eprintln!("Error: FPS not found ({e})");
        StatusCode::INTERNAL_SERVER_ERROR
    })?;

    println!("Exporting video...");
    let path = export::export(format!("imgs/{identifier}-{count}"), fps).await.map_err(|e| {
        eprintln!("Error: Could not export video ({e})");
        StatusCode::INTERNAL_SERVER_ERROR
    })?;

    println!("Finished exporting video {path}");

    Ok(())
}

async fn start(AuthBearer(token): AuthBearer) -> Result<String, StatusCode> {
    let time_str = Local::now().format("%y-%m-%d");
    let mut rng = rand::thread_rng();
    let identifier = format!("{time_str}_{}", rng.gen_range(100..999));

    // let auth = headers.get("Authorisation").ok_or_else(|| {
    //     eprintln!("Error: Authorisation header not found");
    //     StatusCode::UNAUTHORIZED
    // })?.to_str().map_err(|e| {
    //     eprintln!("Error: Authorisation header is not a string ({e})");
    //     StatusCode::BAD_REQUEST
    // })?;

    let secret = env::var("SECRET").map_err(|e| {
        eprintln!("Error: SECRET not found ({e})");
        StatusCode::INTERNAL_SERVER_ERROR
    })?;

    if token != secret {
        eprintln!("Error: Authorisation is not correct ({token})");
        return Err(StatusCode::UNAUTHORIZED);
    }

    std::fs::create_dir(format!("imgs/{identifier}")).map_err(|e| {
        eprintln!("Error: Could not create folder ({e})");
        StatusCode::INTERNAL_SERVER_ERROR
    })?;

    println!("Created new folder: {identifier}");
    Ok(identifier)
}

// #[debug_handler]
async fn upload(AuthBearer(token): AuthBearer, Query(params): Query<Params>, body: Bytes) -> impl IntoResponse {
    let time_str = Local::now().format("%y-%m-%d_%H:%M:%S");

    let secret = env::var("SECRET").map_err(|e| {
        eprintln!("Error: SECRET not found ({e})");
        StatusCode::INTERNAL_SERVER_ERROR
    })?;

    if token != secret {
        eprintln!("Error: Authorisation is not correct ({token})");
        return Err(StatusCode::UNAUTHORIZED);
    }

    let count = params.count.ok_or_else(|| {
        eprintln!("Error: Count param not found");
        StatusCode::BAD_REQUEST
    })?;

    let identifier = params.identifier.ok_or_else(|| {
        eprintln!("Error: Identifier param not found");
        StatusCode::BAD_REQUEST
    })?;

    let mut file = File::create(format!("imgs/{identifier}/{count}_{time_str}.jpg")).await.map_err(|e| {
        eprintln!("Error: Could not create file ({e})");
        StatusCode::INTERNAL_SERVER_ERROR
    })?;

    file.write_all(&body).await.map_err(|e| {
        eprintln!("Error: Could not write to file ({e})");
        StatusCode::INTERNAL_SERVER_ERROR
    })?;

    println!("Saved image: {time_str}.{count}.jpg");
    Ok(())
}
