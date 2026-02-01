#!/usr/bin/env bash
set -e
mkdir -p /images
chown -R app:app /images || true

exec setpriv --reuid=app --regid=app --init-groups "$@"

