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
        .route("/", get(index))
        .route("/upload", post(upload))
        .route("/finish", get(finish))
        .route("/latest-image", get(latest_image))
        .route("/final-video", get(final_video))
        .route("/start", get(start));

    let listener = tokio::net::TcpListener::bind("0.0.0.0:8080")
        .await
        .expect("server failed to start");
    log::info!("listening on {:?}", listener);
    axum::serve(listener, app).await.unwrap();
}

async fn test() {
    let id = Command::new("id").output().await.expect("Cant run id");
    log::debug!(
        "id: {}",
        String::from_utf8_lossy(&id.stdout)
            .strip_suffix("\n")
            .unwrap_or("")
    );

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
    let time_str = Local::now().format("%y-%m-%d-%H");
    let mut rng = rand::rng();
    let identifier = format!("{time_str}_{}", rng.random_range(00..99));

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
    log::info!("Received image: ({time_str}) for params: {params:?}");

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
    if body.len() < 100 {
        log::warn!("Error: Image data too small");
        return Err(StatusCode::BAD_REQUEST.into());
    }

    let img_type = if layer == "1" {
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
        LAYER_NAME,
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
        MINUTE_NAME,
    )
    .await
    .map_err(|e| {
        log::warn!("Error: Could not export video 2 ({e})");
        StatusCode::INTERNAL_SERVER_ERROR
    })?;

    log::info!("Finished exporting video {layer_path} and {minute_path}");
    Ok::<(), axum::response::ErrorResponse>(())
}

#[derive(Debug, Deserialize)]
struct LatestParams {
    identifier: Option<String>,
    layer: Option<String>,
}

#[debug_handler]
async fn latest_image(Query(params): Query<LatestParams>) -> impl IntoResponse {
    let identifier = if let Some(id) = params.identifier {
        id
    } else {
        // choose the last identifier (latest directory) under `images`
        let mut last_id: Option<String> = None;
        let read_root = fs::read_dir(FOLDER_NAME).map_err(|e| {
            log::warn!("Error: Could not read folder ({e})");
            StatusCode::NOT_FOUND
        })?;

        for entry in read_root {
            let entry = entry.map_err(|e| {
                log::warn!("Error: Could not read directory entry ({e})");
                StatusCode::INTERNAL_SERVER_ERROR
            })?;
            let file_type = entry.file_type().map_err(|e| {
                log::warn!("Error: Could not get file type ({e})");
                StatusCode::INTERNAL_SERVER_ERROR
            })?;
            if !file_type.is_dir() {
                continue;
            }
            let name = entry.file_name().to_string_lossy().to_string();
            // skip already finished folders
            if last_id.as_ref().map_or(true, |f| name > *f) {
                last_id = Some(name);
            }
        }

        last_id.ok_or_else(|| {
            log::warn!("Error: No identifiers found");
            StatusCode::NOT_FOUND
        })?
    };

    let layer = params.layer.unwrap_or_else(|| "false".to_string());
    let img_type = if layer == "1" {
        LAYER_NAME
    } else {
        MINUTE_NAME
    };

    let dir = format!("{FOLDER_NAME}/{identifier}");
    log::debug!("Reading images from: {dir}");

    let mut latest_file: Option<String> = None;
    let read_dir = fs::read_dir(&dir).map_err(|e| {
        log::warn!("Error: Could not read folder ({e})");
        StatusCode::NOT_FOUND
    })?;

    for entry in read_dir {
        let entry = entry.map_err(|e| {
            log::warn!("Error: Could not read directory entry ({e})");
            StatusCode::INTERNAL_SERVER_ERROR
        })?;
        let name = entry.file_name().to_string_lossy().to_string();
        if name.starts_with(&format!("{img_type}_")) && name.ends_with(".jpg") {
            if latest_file.as_ref().map_or(true, |f| name > *f) {
                latest_file = Some(name);
            }
        }
    }

    let fname = latest_file.ok_or_else(|| {
        log::warn!("Error: No images found");
        StatusCode::NOT_FOUND
    })?;

    let path = format!("{dir}/{fname}");
    log::debug!("Reading image from: {path}");
    let data = tokio::fs::read(&path).await.map_err(|e| {
        log::warn!("Error: Could not read file ({path}) ({e})");
        StatusCode::INTERNAL_SERVER_ERROR
    })?;

    Ok::<_, StatusCode>((
        StatusCode::OK,
        [("content-type", "image/jpeg")],
        Bytes::from(data),
    ))
}

#[debug_handler]
async fn final_video(Query(params): Query<LatestParams>) -> impl IntoResponse {
    let identifier = if let Some(id) = params.identifier {
        id
    } else {
        let mut last_id: Option<String> = None;
        let read_root = fs::read_dir(FOLDER_NAME).map_err(|e| {
            log::warn!("Error: Could not read folder ({e})");
            StatusCode::NOT_FOUND
        })?;

        for entry in read_root {
            let entry = entry.map_err(|e| {
                log::warn!("Error: Could not read directory entry ({e})");
                StatusCode::INTERNAL_SERVER_ERROR
            })?;
            let file_type = entry.file_type().map_err(|e| {
                log::warn!("Error: Could not get file type ({e})");
                StatusCode::INTERNAL_SERVER_ERROR
            })?;
            if !file_type.is_dir() {
                continue;
            }
            let name = entry.file_name().to_string_lossy().to_string();
            if last_id.as_ref().map_or(true, |f| name > *f) {
                last_id = Some(name);
            }
        }

        last_id.ok_or_else(|| {
            log::warn!("Error: No identifiers found");
            StatusCode::NOT_FOUND
        })?
    };

    let layer = params.layer.unwrap_or_else(|| "false".to_string());
    let video_name = if layer == "1" {
        LAYER_NAME
    } else {
        MINUTE_NAME
    };

    let dir = format!("{FOLDER_NAME}/{identifier}");
    log::debug!("Reading videos from: {dir}");

    let mut latest_file: Option<String> = None;
    let read_dir = fs::read_dir(&dir).map_err(|e| {
        log::warn!("Error: Could not read folder ({e})");
        StatusCode::NOT_FOUND
    })?;

    for entry in read_dir {
        let entry = entry.map_err(|e| {
            log::warn!("Error: Could not read directory entry ({e})");
            StatusCode::INTERNAL_SERVER_ERROR
        })?;
        let name = entry.file_name().to_string_lossy().to_string();
        if name.starts_with(video_name) && name.ends_with(".mp4") {
            if latest_file.as_ref().map_or(true, |f| name > *f) {
                latest_file = Some(name);
            }
        }
    }

    let fname = latest_file.ok_or_else(|| {
        log::warn!("Error: No videos found");
        StatusCode::NOT_FOUND
    })?;

    let path = format!("{dir}/{fname}");
    log::debug!("Reading video from: {path}");
    let data = tokio::fs::read(&path).await.map_err(|e| {
        log::warn!("Error: Could not read file ({path}) ({e})");
        StatusCode::INTERNAL_SERVER_ERROR
    })?;

    Ok::<_, StatusCode>((
        StatusCode::OK,
        [("content-type", "video/mp4")],
        Bytes::from(data),
    ))
}

#[debug_handler]
async fn index() -> impl IntoResponse {
    fn html_escape(s: &str) -> String {
        s.replace('&', "&amp;")
            .replace('<', "&lt;")
            .replace('>', "&gt;")
            .replace('"', "&quot;")
    }

    let mut ids: Vec<String> = Vec::new();
    let read_root = fs::read_dir(FOLDER_NAME).map_err(|e| {
        log::warn!("Error: Could not read folder ({e})");
        StatusCode::NOT_FOUND
    })?;

    for entry in read_root {
        let entry = entry.map_err(|e| {
            log::warn!("Error: Could not read directory entry ({e})");
            StatusCode::INTERNAL_SERVER_ERROR
        })?;
        let file_type = entry.file_type().map_err(|e| {
            log::warn!("Error: Could not get file type ({e})");
            StatusCode::INTERNAL_SERVER_ERROR
        })?;
        if !file_type.is_dir() {
            continue;
        }
        let name = entry.file_name().to_string_lossy().to_string();
        ids.push(name);
    }
    ids.sort();
    ids.reverse();

    let mut html = String::new();
    html.push_str("<!doctype html><html><head><meta charset=\"utf-8\"><title>Identifiers</title></head><body>");
    html.push_str("<h1>Identifiers</h1><ul>");

    for id in ids {
        let disp = html_escape(&id);
        // use raw id in query params (identifiers are expected to be filesystem-safe like `yy-mm-dd-HH_xx`)
        html.push_str(&format!(
            "<li><strong>{}</strong> — <a href=\"/latest-image?identifier={}&layer=true\">last layer image</a> | <a href=\"/latest-image?identifier={}&layer=false\">last minute image</a>",
            disp, id, id
        ));
        if id.contains("-finished-") {
            html.push_str(&format!(
                " | <a href=\"/final-video?identifier={}&layer=true\">layer video</a> | <a href=\"/final-video?identifier={}&layer=false\">minute video</a>",
                id, id
            ));
        }
        html.push_str("</li>");
    }

    html.push_str("</ul></body></html>");
    Ok::<_, StatusCode>((
        StatusCode::OK,
        [("content-type", "text/html; charset=utf-8")],
        html,
    ))
}
