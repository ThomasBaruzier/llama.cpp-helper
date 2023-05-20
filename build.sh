#!/bin/bash

init() {
  [ -d src ] && return
  [ -d llama.cpp ] && mv llama.cpp src && return
  git clone --depth 1 'https://github.com/ggerganov/llama.cpp' src
}

update() {
  cd src
  if [ ! -x main ]; then
    git pull || echo "Warning : Failed to pull latest update."
    echo "No executable found. Building."
    build; return
  fi
  if [ "$1" = update ]; then
    echo "Checking for updates..."
    [ "$(git pull)" = 'Already up to date.' ] && echo "Already updated." && return
    [ "$?" != "0" ] && echo "Warning : Failed to pull latest update."
    echo "New update found. Building."
    build
  fi
}

build() {
  make clean && make main -j && \
  echo "Successfully updated" || echo "Update failed"
}

init && update "$1"
cd .. && gcc chat.c -o chat -Os -s && echo "Build successful"
