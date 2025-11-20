use std::io::Error;
use tokio::process::Command;

pub async fn export(path: String, fps: String) -> Result<String, Error> {
    let mut cmd = Command::new("ffmpeg");
    cmd.arg("-pattern_type").arg("glob").arg("-framerate").arg(fps).arg("-i").arg(format!("{path}/*.jpg")).arg("-c:v").arg("libx264").arg("-preset").arg("slow").arg("-crf").arg("18").arg(format!("{path}/out.mp4"));
    log::debug!("Running ffmpeg...{cmd:?}");
    cmd.output().await.map_err(|e| {
        log::warn!("Error: Could not run ffmpeg ({e})");
        e
    }).map(|d| {
        log::debug!("Data: {d:?}");
    })?;

    Ok(format!("{path}/out.mp4"))
}