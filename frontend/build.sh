#!/bin/bash

set -e

npm run build

gzip -c -9 ./build/index.html > ../firmware/spiffs/index
gzip -c -9 ./build/favicon.ico > ../firmware/spiffs/favicon
