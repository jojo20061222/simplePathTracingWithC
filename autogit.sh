#!/bin/bash

while true; do
    git add -u
    if ! git diff --cached --quiet; then
        git commit -m "autogit ($(date '+%Y-%m-%d %H:%M:%S'))"
    fi
    sleep 30m
done