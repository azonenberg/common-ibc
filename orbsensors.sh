#!/bin/sh
orbcat --channel "0,%c" --server localhost:6020 %c | socat - TCP4-LISTEN:6021,reuseaddr,fork
