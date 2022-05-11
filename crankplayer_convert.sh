#!/bin/sh

set -eu

mkdir crankplayer_frames
ffmpeg -i "$1" -s 400x240 -filter:v fps=30 crankplayer_frames/frame%06d.pbm # 640K ought to be enough for anybody
./crankplayer_convert.py crankplayer_frames/frame*.pbm > Source/video.pdv
ffmpeg -i "$1" -acodec adpcm_ima_wav Source/audio.wav
ffmpeg -i "$1" -af areverse -acodec adpcm_ima_wav Source/audio_reversed.wav
rm -rf crankplayer_frames
