mod export;

use std::{env, fs};
use axum::{routing::get, routing::post, Router, debug_handler};
use axum::body::Bytes;
use axum::extract::{Query};
use chrono::offset::Local;
use axum::http::{StatusCode};
use axum::response::IntoResponse;
use rand::Rng;
use serde::Deserialize;
use tokio::fs::File;
use tokio::io::AsyncWriteExt;
use tokio::process::Command;

const FOLDER_NAME: &str = "images";

#[tokio::main]
async fn main() {
    test().await;

    let app = Router::new()
        .route("/upload", post(upload))
        .route("/finish", get(finish))
        .route("/start", get(start));

    let listener = tokio::net::TcpListener::bind("0.0.0.0:8080").await.expect("server failed to start");
    println!("listening on {:?}", listener);
    axum::serve(listener, app).await.unwrap();
}

async fn test() {
    env::var("FPS").expect("Error: FPS not found");

    let mut file = File::create(format!("{FOLDER_NAME}/test")).await.expect("Cant create test file");
    file.write_all(b"test").await.expect("Cant write to test file");
    fs::remove_file(format!("{FOLDER_NAME}/test")).expect("Cant remove test file");

    let mut cmd = Command::new("ffmpeg");
    cmd.arg("-h");
    cmd.output().await.expect("Cant run ffmpeg");
}

#[derive(Debug, Deserialize)]
#[allow(dead_code)]
struct Params {
    count: Option<i32>,
    identifier: Option<String>,
}

async fn finish(Query(params): Query<Params>) -> impl IntoResponse {
    let count = params.count.ok_or_else(|| {
        eprintln!("Error: Count param not found");
        StatusCode::BAD_REQUEST
    })?;

    let identifier = params.identifier.ok_or_else(|| {
        eprintln!("Error: Identifier param not found");
        StatusCode::BAD_REQUEST
    })?;

    fs::rename(format!("{FOLDER_NAME}/{identifier}"), format!("{FOLDER_NAME}/{identifier}-finished-{count}")).map_err(|e| {
        eprintln!("Error: Could not rename folder ({e})");
        StatusCode::INTERNAL_SERVER_ERROR
    })?;

    println!("Finished Folder {identifier}-{count}");

    let fps = env::var("FPS").map_err(|e| {
        eprintln!("Error: FPS not found ({e})");
        StatusCode::INTERNAL_SERVER_ERROR
    })?;

    println!("Exporting video...");
    let path = export::export(format!("{FOLDER_NAME}/{identifier}-finished-{count}"), fps).await.map_err(|e| {
        eprintln!("Error: Could not export video ({e})");
        StatusCode::INTERNAL_SERVER_ERROR
    })?;

    println!("Finished exporting video {path}");
    Ok::<(), axum::response::ErrorResponse>(())
}

async fn start() -> Result<String, StatusCode> {
    let time_str = Local::now().format("%y-%m-%d");
    let mut rng = rand::rng();
    let identifier = format!("{time_str}_{}", rng.random_range(100..999));

    fs::create_dir_all(format!("{FOLDER_NAME}/{identifier}")).map_err(|e| {
        eprintln!("Error: Could not create folder ({e})");
        StatusCode::INTERNAL_SERVER_ERROR
    })?;

    println!("Created new folder: {identifier}");
    Ok(identifier)
}

#[debug_handler]
async fn upload(Query(params): Query<Params>, body: Bytes) -> impl IntoResponse {
    let time_str = Local::now().format("%y-%m-%d_%H:%M:%S");
    println!("Received image: ({time_str})");

    let count = params.count.ok_or_else(|| {
        eprintln!("Error: Count param not found");
        StatusCode::BAD_REQUEST
    })?;
    // pad count to 4 digits
    let count = format!("{:04}", count);

    let identifier = params.identifier.ok_or_else(|| {
        eprintln!("Error: Identifier param not found");
        StatusCode::BAD_REQUEST
    })?;

    let mut file = File::create(format!("{FOLDER_NAME}/{identifier}/{count}.jpg")).await.map_err(|e| {
        eprintln!("Error: Could not create file({}) ({e})", format!("{FOLDER_NAME}/{identifier}/{count}.jpg"));
        StatusCode::INTERNAL_SERVER_ERROR
    })?;

    file.write_all(&body).await.map_err(|e| {
        eprintln!("Error: Could not write to file({:?}) ({e})", file);
        StatusCode::INTERNAL_SERVER_ERROR
    })?;

    println!("Saved image: ({time_str}): {count}.jpg");
    Ok::<(), axum::response::ErrorResponse>(())
}
