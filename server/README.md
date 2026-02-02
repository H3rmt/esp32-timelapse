# ESP32 Timelapse Server

A Rust-based server designed to receive images from an ESP32 camera and process them into timelapse videos.

> [!WARNING]
> This project is currently **under development** and is primarily for personal use.

## Features

- **Image Upload**: Receives JPEG images from an ESP32 or other clients.
- **Organization**: Automatically organizes images by identifier and layer/minute categories.
- **Automated Export**: Triggers background FFmpeg processing to generate MP4 videos once a capture session is finished.
- **Status Preview**: View the latest captured image or the final processed videos via API.

## Tech Stack

- **Language**: [Rust](https://www.rust-lang.org/)
- **Web Framework**: [Axum](https://github.com/tokio-rs/axum)
- **Async Runtime**: [Tokio](https://tokio.rs/)
- **Video Processing**: [FFmpeg](https://ffmpeg.org/)

## API Endpoints

- `GET /`: Index page (status overview).
- `GET /start`: Generates a new unique identifier for a capture session.
- `POST /upload`: Upload a JPEG image. Requires `count`, `identifier`, and `layer` query parameters.
- `GET /finish`: Finalize a session and trigger video export. Requires `identifier`, `layer_count`, and `minute_count`.
- `GET /latest-image`: Fetch the most recently uploaded image.
- `GET /final-video`: Fetch the latest generated timelapse video.

## File Storage

Uploaded images and generated videos are stored in the `./images` directory, relative to the server binary.
In the Docker container, the binary is located at the root (`/server`), so files are stored at `/images`.

`/images` is organized by identifier and contains all the jpgs and mp4s for a given capture session.

## Setup & Configuration

The easiest way to run the server is using Docker Compose.

### Environment Variables

| Variable     | Description                                   | Default  |
|--------------|-----------------------------------------------|----------|
| `LAYER_FPS`  | Frames per second for the 'layer' timelapse.  | Required |
| `MINUTE_FPS` | Frames per second for the 'minute' timelapse. | Required |
| `TZ`         | Timezone for file naming.                     | `UTC`    |

### Running with Docker Compose

```yaml
services:
  esp32-timelapse-server:
    image: ghcr.io/h3rmt/esp32-timelapse-server:latest
    restart: always
    ports:
      - "8080:8080"
    environment:
      - LAYER_FPS=14
      - MINUTE_FPS=60
      - TZ=Europe/Berlin
    volumes:
      - ./images:/images
```

Note: The container runs rootless (3000:3000), uses `tini` as PID 1, and on startup will `chown` the mounted `/images` directory to the service user to ensure correct file permissions.
