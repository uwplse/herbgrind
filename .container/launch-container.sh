#!/bin/bash

if ! which podman; then
    >&2 echo "this script expects podman to be installed on the system."
    exit 1
fi

CONTAINER_FOLDER=$(dirname $BASH_SOURCE)
ENV_HASH=$(cat "$CONTAINER_FOLDER"/env/* | openssl sha256 | cut -f2 -d ' ')
IMAGE="localhost/herbgrind:${ENV_HASH:0:8}"
if ! podman image ls | grep $IMAGE; then
   podman build -t $IMAGE "$CONTAINER_FOLDER/env"
fi
podman run \
       -v "$PWD":"$PWD" \
       -e TERM=xterm-256color \
       -w "$PWD" \
       --detach-keys=ctrl-@ \
       --security-opt seccomp=unconfined \
       --rm \
       --name herbgrind \
       -it "$IMAGE" \
       sh -c "cat /etc/motd; bash"
