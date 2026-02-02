mod consts;
mod export;
mod handlers;

use crate::consts::{FOLDER_NAME, LAYER_FPS_NAME, MINUTE_FPS_NAME};
use crate::handlers::{final_video, finish, index, latest_image, start, upload};
use axum::{routing::get, routing::post, Router};
use std::{env, fs};
use tokio::fs::File;
use tokio::io::AsyncWriteExt;
use tokio::process::Command;
use tracing::{debug, info, warn};
use tracing_subscriber::EnvFilter;

#[tokio::main]
async fn main() {
    let filter = EnvFilter::try_from_default_env()
        .unwrap_or_else(|_| "esp_32_img_server=info".to_string().into());
    let subscriber = tracing_subscriber::fmt()
        .with_timer(tracing_subscriber::fmt::time::time())
        .with_target(false)
        .with_env_filter(filter)
        .finish();
    tracing::subscriber::set_global_default(subscriber)
        .unwrap_or_else(|e| warn!("Unable to initialize logging: {e}"));

    test().await;

    let app = Router::new()
        .route("/", get(index))
        .route("/upload", post(upload))
        .route("/finish", get(finish))
        .route("/latest-image", get(latest_image))
        .route("/final-video", get(final_video))
        .route("/start", get(start));

    let listener = tokio::net::TcpListener::bind("0.0.0.0:8080")
        .await
        .expect("server failed to start");
    info!("listening on {:?}", listener.local_addr().unwrap());
    axum::serve(listener, app).await.unwrap();
}

#[tracing::instrument]
async fn test() {
    let id = Command::new("id").output().await.expect("Cant run id");
    debug!(
        "id: {}",
        String::from_utf8_lossy(&id.stdout)
            .strip_suffix("\n")
            .unwrap_or("")
    );

    env::var(LAYER_FPS_NAME).unwrap_or_else(|_| panic!("Error: {LAYER_FPS_NAME} not found"));
    debug!("LAYER_FPS_NAME env found");
    env::var(MINUTE_FPS_NAME).unwrap_or_else(|_| panic!("Error: {MINUTE_FPS_NAME} not found"));
    debug!("MINUTE_FPS_NAME env found");

    fs::create_dir_all(FOLDER_NAME).expect("Cant create folder");
    let mut file = File::create(format!("{FOLDER_NAME}/test"))
        .await
        .expect("Cant create test file");
    file.write_all(b"test")
        .await
        .expect("Cant write to test file");
    fs::remove_file(format!("{FOLDER_NAME}/test")).expect("Cant remove test file");
    debug!("test file created");

    fs::create_dir_all(format!("{FOLDER_NAME}/00000000000000"))
        .expect("Cant create fallback folder");

    let mut cmd = Command::new("ffmpeg");
    cmd.arg("-h");
    cmd.output().await.expect("Cant run ffmpeg");
    debug!("ffmpeg found");
}
