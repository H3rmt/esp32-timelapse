mod export;

use axum::body::Bytes;
use axum::extract::Query;
use axum::http::StatusCode;
use axum::response::IntoResponse;
use axum::{routing::get, routing::post, Router};
use chrono::offset::Local;
use rand::Rng;
use serde::Deserialize;
use std::{env, fs};
use tokio::fs::File;
use tokio::io::AsyncWriteExt;
use tokio::process::Command;
use tracing::{debug, info, warn};
use tracing_subscriber::EnvFilter;

const LAYER_FPS_NAME: &str = "LAYER_FPS";
const MINUTE_FPS_NAME: &str = "MINUTE_FPS";

const FOLDER_NAME: &str = "images";

const LAYER_NAME: &str = "layer";
const MINUTE_NAME: &str = "minute";

#[tokio::main]
async fn main() {
    let filter = EnvFilter::try_from_default_env()
        .unwrap_or_else(|_| "esp_32_img_server=info".to_string().into());
    let subscriber = tracing_subscriber::fmt()
        .with_timer(tracing_subscriber::fmt::time::time())
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

    env::var(LAYER_FPS_NAME).expect(&format!("Error: {LAYER_FPS_NAME} not found"));
    debug!("LAYER_FPS_NAME env found");
    env::var(MINUTE_FPS_NAME).expect(&format!("Error: {MINUTE_FPS_NAME} not found"));
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

#[tracing::instrument]
async fn start() -> Result<String, StatusCode> {
    let time_str = Local::now().format("%y-%m-%d-%H");
    let mut rng = rand::rng();
    let identifier = format!("{time_str}_{}", rng.random_range(00..99));

    fs::create_dir_all(format!("{FOLDER_NAME}/{identifier}")).map_err(|e| {
        warn!("Error: Could not create folder ({e})");
        StatusCode::INTERNAL_SERVER_ERROR
    })?;

    info!("Created new folder: {identifier}");
    Ok(identifier)
}

#[derive(Debug, Deserialize)]
struct UploadParams {
    count: Option<i32>,
    identifier: Option<String>,
    layer: Option<String>,
}

#[tracing::instrument(skip(body))]
async fn upload(Query(params): Query<UploadParams>, body: Bytes) -> impl IntoResponse {
    let time_str = Local::now().format("%y-%m-%d_%H:%M:%S");
    info!("Received image: ({time_str}) for params: {params:?}");

    let count = params.count.ok_or_else(|| {
        warn!("Error: Count param not found");
        StatusCode::BAD_REQUEST
    })?;
    // pad count to 5 digits
    let count = format!("{:05}", count);

    let identifier = params.identifier.ok_or_else(|| {
        warn!("Error: Identifier param not found");
        StatusCode::BAD_REQUEST
    })?;
    let layer = params.layer.ok_or_else(|| {
        warn!("Error: Layer param not found");
        StatusCode::BAD_REQUEST
    })?;
    if body.len() < 100 {
        warn!("Error: Image data too small");
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
            warn!(
                "Error: Could not create file({}) ({e})",
                format!("{FOLDER_NAME}/{identifier}/{img_type}_{count}.jpg")
            );
            StatusCode::INTERNAL_SERVER_ERROR
        })?;

    file.write_all(&body).await.map_err(|e| {
        warn!("Error: Could not write to file({file:?}) ({e})");
        StatusCode::INTERNAL_SERVER_ERROR
    })?;

    info!("Saved image: ({time_str}): {img_type}_{count}.jpg");
    Ok::<(), axum::response::ErrorResponse>(())
}

#[derive(Debug, Deserialize)]
struct FinishParams {
    layer_count: Option<i32>,
    minute_count: Option<i32>,
    identifier: Option<String>,
}

#[tracing::instrument]
async fn finish(Query(params): Query<FinishParams>) -> impl IntoResponse {
    let identifier = params.identifier.ok_or_else(|| {
        warn!("Error: Identifier param not found");
        StatusCode::BAD_REQUEST
    })?;
    let layer_count = params.layer_count.ok_or_else(|| {
        warn!("Error: Count param not found");
        StatusCode::BAD_REQUEST
    })?;
    let minute_count = params.minute_count.ok_or_else(|| {
        warn!("Error: minute_count param not found");
        StatusCode::BAD_REQUEST
    })?;

    fs::rename(
        format!("{FOLDER_NAME}/{identifier}"),
        format!("{FOLDER_NAME}/{identifier}-finished-{layer_count}-{minute_count}"),
    )
    .map_err(|e| {
        warn!("Error: Could not rename folder ({e})");
        StatusCode::INTERNAL_SERVER_ERROR
    })?;

    info!("Finished Folder {identifier}-{layer_count}-{minute_count}");

    let layer_fps = env::var("LAYER_FPS").map_err(|e| {
        warn!("Error: FPS not found ({e})");
        StatusCode::INTERNAL_SERVER_ERROR
    })?;
    let minute_fps = env::var("MINUTE_FPS").map_err(|e| {
        warn!("Error: FPS not found ({e})");
        StatusCode::INTERNAL_SERVER_ERROR
    })?;

    info!("Scheduling background export for videos...");
    let base_path = format!("{FOLDER_NAME}/{identifier}-finished-{layer_count}-{minute_count}");
    let layer_fps_cloned = layer_fps.clone();
    let minute_fps_cloned = minute_fps.clone();
    let base_for_layer = base_path.clone();
    let base_for_minute = base_path.clone();

    tokio::spawn(async move {
        match export::export(
            &base_for_layer,
            &layer_fps_cloned,
            &format!("{LAYER_NAME}_*.jpg"),
            LAYER_NAME,
        )
        .await
        {
            Ok(path) => info!("Background exported layer video: {path}"),
            Err(e) => warn!("Error: Could not export layer video in background ({e})"),
        }
        match export::export(
            &base_for_minute,
            &minute_fps_cloned,
            &format!("{MINUTE_NAME}_*.jpg"),
            MINUTE_NAME,
        )
        .await
        {
            Ok(path) => info!("Background exported minute video: {path}"),
            Err(e) => warn!("Error: Could not export minute video in background ({e})"),
        }
    });

    info!("Export jobs started in background for {base_path}");
    Ok::<(), axum::response::ErrorResponse>(())
}

#[derive(Debug, Deserialize)]
struct LatestParams {
    identifier: Option<String>,
    layer: Option<String>,
}

#[tracing::instrument]
async fn latest_image(Query(params): Query<LatestParams>) -> impl IntoResponse {
    let identifier = if let Some(id) = params.identifier {
        id
    } else {
        // choose the last identifier (latest directory) under `images`
        let mut last_id: Option<String> = None;
        let read_root = fs::read_dir(FOLDER_NAME).map_err(|e| {
            warn!("Error: Could not read folder ({e})");
            StatusCode::NOT_FOUND
        })?;

        for entry in read_root {
            let entry = entry.map_err(|e| {
                warn!("Error: Could not read directory entry ({e})");
                StatusCode::INTERNAL_SERVER_ERROR
            })?;
            let file_type = entry.file_type().map_err(|e| {
                warn!("Error: Could not get file type ({e})");
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
            warn!("Error: No identifiers found");
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
    debug!("Reading images from: {dir}");

    let mut latest_file: Option<String> = None;
    let read_dir = fs::read_dir(&dir).map_err(|e| {
        warn!("Error: Could not read folder ({e})");
        StatusCode::NOT_FOUND
    })?;

    for entry in read_dir {
        let entry = entry.map_err(|e| {
            warn!("Error: Could not read directory entry ({e})");
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
        warn!("Error: No images found");
        StatusCode::NOT_FOUND
    })?;

    let path = format!("{dir}/{fname}");
    debug!("Reading image from: {path}");
    let data = tokio::fs::read(&path).await.map_err(|e| {
        warn!("Error: Could not read file ({path}) ({e})");
        StatusCode::INTERNAL_SERVER_ERROR
    })?;

    Ok::<_, StatusCode>((
        StatusCode::OK,
        [("content-type", "image/jpeg")],
        Bytes::from(data),
    ))
}

#[tracing::instrument]
async fn final_video(Query(params): Query<LatestParams>) -> impl IntoResponse {
    let identifier = if let Some(id) = params.identifier {
        id
    } else {
        let mut last_id: Option<String> = None;
        let read_root = fs::read_dir(FOLDER_NAME).map_err(|e| {
            warn!("Error: Could not read folder ({e})");
            StatusCode::NOT_FOUND
        })?;

        for entry in read_root {
            let entry = entry.map_err(|e| {
                warn!("Error: Could not read directory entry ({e})");
                StatusCode::INTERNAL_SERVER_ERROR
            })?;
            let file_type = entry.file_type().map_err(|e| {
                warn!("Error: Could not get file type ({e})");
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
            warn!("Error: No identifiers found");
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
    debug!("Reading videos from: {dir}");

    let mut latest_file: Option<String> = None;
    let read_dir = fs::read_dir(&dir).map_err(|e| {
        warn!("Error: Could not read folder ({e})");
        StatusCode::NOT_FOUND
    })?;

    for entry in read_dir {
        let entry = entry.map_err(|e| {
            warn!("Error: Could not read directory entry ({e})");
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
        warn!("Error: No videos found");
        StatusCode::NOT_FOUND
    })?;

    let path = format!("{dir}/{fname}");
    debug!("Reading video from: {path}");
    let data = tokio::fs::read(&path).await.map_err(|e| {
        warn!("Error: Could not read file ({path}) ({e})");
        StatusCode::INTERNAL_SERVER_ERROR
    })?;

    Ok::<_, StatusCode>((
        StatusCode::OK,
        [("content-type", "video/mp4")],
        Bytes::from(data),
    ))
}

async fn index() -> impl IntoResponse {
    let mut ids: Vec<(String, u16, u16)> = Vec::new();
    let read_root = fs::read_dir(FOLDER_NAME).map_err(|e| {
        warn!("Error: Could not read folder ({e})");
        StatusCode::NOT_FOUND
    })?;

    for entry in read_root {
        let entry = entry.map_err(|e| {
            warn!("Error: Could not read directory entry ({e})");
            StatusCode::INTERNAL_SERVER_ERROR
        })?;
        let file_type = entry.file_type().map_err(|e| {
            warn!("Error: Could not get file type ({e})");
            StatusCode::INTERNAL_SERVER_ERROR
        })?;
        if !file_type.is_dir() {
            continue;
        }
        let name = entry.file_name().to_string_lossy().to_string();
        let mut layers = 0;
        let mut minutes = 0;
        if let Ok(sub_read) = fs::read_dir(entry.path()) {
            for sub_ent in sub_read.flatten() {
                if let Ok(sub_ft) = sub_ent.file_type() {
                    if sub_ft.is_file() {
                        let img_name = sub_ent.file_name().to_string_lossy().to_string();
                        if img_name.starts_with(LAYER_NAME) {
                            layers += 1;
                        } else if img_name.starts_with(MINUTE_NAME) {
                            minutes += 1;
                        } else {
                            warn!("Error: Unknown image type ({img_name})");
                        }
                    }
                }
            }
        } else {
            warn!("Error: Could not read subfolder for validation ({})", name);
        }
        ids.push((name, layers, minutes));
    }
    ids.sort_by(|a, b| b.0.cmp(&a.0));

    let mut html = String::from("<!doctype html><html><head><meta charset=\"utf-8\"><title>Identifiers</title><style>\
body{font-family:Arial,Helvetica,sans-serif;font-size:1.8rem;display:flex;flex-direction:column;align-items:center;justify-content:flex-start;height:calc(100dvh - 40px);padding:20px;margin:0;background:#ffffff;color:#000000}\
h1{margin:0 0 12px 0;padding:0}\
ul{list-style:none;padding:0 1rem;margin:0;overflow:auto}\
li{margin:0;padding:8px 0;border-bottom:1px solid #eee;display:flex;align-items:center;gap:1rem}\
span{font-weight:700;min-width:19ch;display:inline-block}\
a{text-decoration:none;color:#0366d6}\
a:hover{text-decoration:underline}\
.grey{color:grey}
.green{color:green}
@media (prefers-color-scheme:dark){body{background:#0b1117;color:#c9d1d9}li{border-bottom:1px solid #222}a{color:#58a6ff}}\
/* Custom scrollbars */\
::-webkit-scrollbar { width: 12px; height: 12px; }\
::-webkit-scrollbar-track { background: transparent; border-radius: 8px; }\
* { scrollbar-color: rgba(15,23,42,0.5) transparent; }\
@media (prefers-color-scheme:dark){ \
  * { scrollbar-color: rgba(255,255,255,0.12) transparent; } \
}\
</style></head><body><h1>Identifiers</h1><ul>");

    for (id, layers, minutes) in ids {
        // use raw id in query params (identifiers are expected to be filesystem-safe like `yy-mm-dd-HH_xx`)
        html.push_str(&format!(
            "<li><span {}>{id}{}</span> &gt; <a href=\"/latest-image?identifier={id}&layer=1\">last layer image</a> | <a href=\"/latest-image?identifier={id}&layer=0\">last minute image</a>",
            if layers + minutes != 0 { "" } else if id.contains("-finished-") {"class=\"green\""} else  { "class=\"grey\"" },
            if layers + minutes != 0 { format!(" ({layers}/{minutes})") } else { "".to_string() }
        ));
        if id.contains("-finished-") {
            html.push_str(&format!(
                " | <a href=\"/final-video?identifier={id}&layer=1\">layer video</a> | <a href=\"/final-video?identifier={id}&layer=0\">minute video</a>",
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
