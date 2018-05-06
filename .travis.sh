#!/usr/bin/env bash

set -ev

opam init --yes --no-setup --comp 4.06.1
eval $(opam config env)

make V=0 compile
make -C bench gromacs-bondfree-580.s
cat bench/gromacs-bondfree-580.s
ocaml --version
make test
