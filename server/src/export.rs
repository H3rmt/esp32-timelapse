use std::io::Error;
use tokio::process::Command;
use tracing::debug;
use tracing::log::warn;

pub async fn export(path: &str, fps: &str, name_reg: &str, out: &str) -> Result<String, Error> {
    let mut cmd = Command::new("ffmpeg");
    cmd.arg("-pattern_type").arg("glob").arg("-framerate").arg(fps).arg("-i").arg(format!("{path}/{name_reg}")).arg("-c:v").arg("libx264").arg("-preset").arg("slow").arg("-crf").arg("18").arg(format!("{path}/{out}.mp4"));
    debug!("Running ffmpeg...{cmd:?}");
    cmd.output().await.map_err(|e| {
        warn!("Error: Could not run ffmpeg ({e})");
        e
    }).map(|d| {
        debug!("Data: {d:?}");
    })?;

    Ok(format!("{path}/out.mp4"))
}