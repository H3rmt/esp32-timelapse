mod export;

use axum::body::Bytes;
use axum::extract::Query;
use axum::http::StatusCode;
use axum::response::IntoResponse;
use axum::{debug_handler, routing::get, routing::post, Router};
use chrono::offset::Local;
use rand::Rng;
use serde::Deserialize;
use std::{env, fs};
use tokio::fs::File;
use tokio::io::AsyncWriteExt;
use tokio::process::Command;

const LAYER_FPS_NAME: &str = "LAYER_FPS";
const MINUTE_FPS_NAME: &str = "MINUTE_FPS";

const FOLDER_NAME: &str = "images";

const LAYER_NAME: &str = "layer";
const MINUTE_NAME: &str = "minute";

#[tokio::main]
async fn main() {
    env_logger::init();

    test().await;

    let app = Router::new()
        .route("/upload", post(upload))
        .route("/finish", get(finish))
        .route("/start", get(start));

    let listener = tokio::net::TcpListener::bind("0.0.0.0:8080")
        .await
        .expect("server failed to start");
    log::info!("listening on {:?}", listener);
    axum::serve(listener, app).await.unwrap();
}

async fn test() {
    env::var(LAYER_FPS_NAME).expect(&format!("Error: {LAYER_FPS_NAME} not found"));
    log::debug!("LAYER_FPS_NAME env found");
    env::var(MINUTE_FPS_NAME).expect(&format!("Error: {MINUTE_FPS_NAME} not found"));
    log::debug!("MINUTE_FPS_NAME env found");

    fs::create_dir_all(FOLDER_NAME).expect("Cant create folder");
    let mut file = File::create(format!("{FOLDER_NAME}/test"))
        .await
        .expect("Cant create test file");
    file.write_all(b"test")
        .await
        .expect("Cant write to test file");
    fs::remove_file(format!("{FOLDER_NAME}/test")).expect("Cant remove test file");
    log::debug!("test file created");

    fs::create_dir_all(format!("{FOLDER_NAME}/00000000000000"))
        .expect("Cant create fallback folder");

    let mut cmd = Command::new("ffmpeg");
    cmd.arg("-h");
    cmd.output().await.expect("Cant run ffmpeg");
    log::debug!("ffmpeg found");
}

async fn start() -> Result<String, StatusCode> {
    let time_str = Local::now().format("%y-%m-%d");
    let mut rng = rand::rng();
    let identifier = format!("{time_str}_{}", rng.random_range(100..999));

    fs::create_dir_all(format!("{FOLDER_NAME}/{identifier}")).map_err(|e| {
        log::warn!("Error: Could not create folder ({e})");
        StatusCode::INTERNAL_SERVER_ERROR
    })?;

    log::info!("Created new folder: {identifier}");
    Ok(identifier)
}

#[derive(Debug, Deserialize)]
struct UploadParams {
    count: Option<i32>,
    identifier: Option<String>,
    layer: Option<String>,
}

#[debug_handler]
async fn upload(Query(params): Query<UploadParams>, body: Bytes) -> impl IntoResponse {
    let time_str = Local::now().format("%y-%m-%d_%H:%M:%S");
    log::info!("Received image: ({time_str})");

    let count = params.count.ok_or_else(|| {
        log::warn!("Error: Count param not found");
        StatusCode::BAD_REQUEST
    })?;
    // pad count to 5 digits
    let count = format!("{:05}", count);

    let identifier = params.identifier.ok_or_else(|| {
        log::warn!("Error: Identifier param not found");
        StatusCode::BAD_REQUEST
    })?;
    let layer = params.layer.ok_or_else(|| {
        log::warn!("Error: Layer param not found");
        StatusCode::BAD_REQUEST
    })?;
    let img_type = if layer == "true" {
        LAYER_NAME
    } else {
        MINUTE_NAME
    };
    let mut file = File::create(format!("{FOLDER_NAME}/{identifier}/{img_type}_{count}.jpg"))
        .await
        .map_err(|e| {
            log::warn!(
                "Error: Could not create file({}) ({e})",
                format!("{FOLDER_NAME}/{identifier}/{img_type}_{count}.jpg")
            );
            StatusCode::INTERNAL_SERVER_ERROR
        })?;

    file.write_all(&body).await.map_err(|e| {
        log::warn!("Error: Could not write to file({file:?}) ({e})");
        StatusCode::INTERNAL_SERVER_ERROR
    })?;

    log::info!("Saved image: ({time_str}): {img_type}_{count}.jpg");
    Ok::<(), axum::response::ErrorResponse>(())
}

#[derive(Debug, Deserialize)]
struct FinishParams {
    layer_count: Option<i32>,
    minute_count: Option<i32>,
    identifier: Option<String>,
}

async fn finish(Query(params): Query<FinishParams>) -> impl IntoResponse {
    let identifier = params.identifier.ok_or_else(|| {
        log::warn!("Error: Identifier param not found");
        StatusCode::BAD_REQUEST
    })?;
    let layer_count = params.layer_count.ok_or_else(|| {
        log::warn!("Error: Count param not found");
        StatusCode::BAD_REQUEST
    })?;
    let minute_count = params.minute_count.ok_or_else(|| {
        log::warn!("Error: minute_count param not found");
        StatusCode::BAD_REQUEST
    })?;

    fs::rename(
        format!("{FOLDER_NAME}/{identifier}"),
        format!("{FOLDER_NAME}/{identifier}-finished-{layer_count}-{minute_count}"),
    )
    .map_err(|e| {
        log::warn!("Error: Could not rename folder ({e})");
        StatusCode::INTERNAL_SERVER_ERROR
    })?;

    log::info!("Finished Folder {identifier}-{layer_count}-{minute_count}");

    let layer_fps = env::var("LAYER_FPS").map_err(|e| {
        log::warn!("Error: FPS not found ({e})");
        StatusCode::INTERNAL_SERVER_ERROR
    })?;
    let minute_fps = env::var("MINUTE_FPS").map_err(|e| {
        log::warn!("Error: FPS not found ({e})");
        StatusCode::INTERNAL_SERVER_ERROR
    })?;

    log::info!("Exporting video 1...");
    let layer_path = export::export(
        &format!("{FOLDER_NAME}/{identifier}-finished-{layer_count}-{minute_count}"),
        &layer_fps,
        &format!("{LAYER_NAME}_*.jpg"),
    )
    .await
    .map_err(|e| {
        log::warn!("Error: Could not export video 1 ({e})");
        StatusCode::INTERNAL_SERVER_ERROR
    })?;

    log::info!("Exporting video 2...");
    let minute_path = export::export(
        &format!("{FOLDER_NAME}/{identifier}-finished-{layer_count}-{minute_count}"),
        &minute_fps,
        &format!("{MINUTE_NAME}_*.jpg"),
    )
    .await
    .map_err(|e| {
        log::warn!("Error: Could not export video 2 ({e})");
        StatusCode::INTERNAL_SERVER_ERROR
    })?;

    log::info!("Finished exporting video {layer_path} and {minute_path}");
    Ok::<(), axum::response::ErrorResponse>(())
}
