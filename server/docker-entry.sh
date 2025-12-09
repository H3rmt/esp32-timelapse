#!/bin/sh
set -e
mkdir -p /images
chown -R app:app /images || true

exec setpriv --reuid=app --regid=app --init-groups "$@"

