#!/bin/bash
set -ex
cd /opt/herbgrind 
eval $(opam config env)
make test
