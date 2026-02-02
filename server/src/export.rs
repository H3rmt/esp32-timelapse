use std::env;
use std::io::Error;
use tokio::process::Command;
use tracing::debug;
use tracing::log::warn;

#[tracing::instrument]
pub async fn export(path: &str, fps: &str, name_reg: &str, out: &str) -> Result<String, Error> {
    let rotation = env::var("ROTATION").unwrap_or_else(|_| "0".to_string());
    let mut cmd = Command::new("ffmpeg");
    cmd.arg("-pattern_type")
        .arg("glob")
        .arg("-framerate")
        .arg(fps)
        .arg("-i")
        .arg(format!("{path}/{name_reg}"))
        .arg("-c:v")
        .arg("libx264")
        .arg("-crf")
        .arg("18");

    if rotation == "90" {
        cmd.arg("-vf").arg("transpose=1");
    } else if rotation == "180" {
        cmd.arg("-vf").arg("transpose=2,transpose=2");
    } else if rotation == "270" {
        cmd.arg("-vf").arg("transpose=2");
    } else if rotation != "0" {
        warn!("Invalid rotation value: {rotation}");
    }

    cmd.arg("-preset")
        .arg("slow")
        .arg("-pix_fmt")
        .arg("yuv420p")
        .arg("-profile:v")
        .arg("high")
        .arg("-level")
        .arg("4.0")
        .arg("-movflags")
        .arg("+faststart")
        .arg(format!("{path}/{out}.mp4"));
    debug!("Running ffmpeg...{cmd:?}");
    cmd.output()
        .await
        .map_err(|e| {
            warn!("Error: Could not run ffmpeg ({e})");
            e
        })
        .map(|d| {
            debug!("Data: {d:?}");
        })?;

    Ok(format!("{path}/out.mp4"))
}
