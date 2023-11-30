use std::env;
use std::io::Error;
use tokio::process::Command;


pub async fn export(path: String, fps: String) -> Result<String, Error> {
    let mut cmd = Command::new("ffmpeg");
    cmd.arg("-framerate").arg(fps).arg("-pattern_type").arg("glob").arg("-i").arg(format!("{path}/*.jpg")).arg("-c:v").arg("libx265").arg("-pix_fmt").arg("yuv420p").arg(format!("{path}/out.mp4"));
    println!("Running ffmpeg...{cmd:?}");
    cmd.output().await.map_err(|e| {
        eprintln!("Error: Could not run ffmpeg ({e})");
        e
    }).map(|d| {
    	println!("Data: {d:?}");
    })?;

    Ok(format!("{path}/out.mp4"))
}
