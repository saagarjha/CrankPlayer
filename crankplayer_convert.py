#!/usr/bin/env python3

import pathlib
import struct
import sys
import zlib

WIDTH = 400
HEIGHT = 240

def read_frame(frame):
	file = pathlib.Path(frame).read_bytes()
	magic = file.find(ord("\n")) + 1
	header = file.find(ord("\n"), magic)
	assert(file[:magic].decode().strip() == "P4")
	(width, height) = map(int, file[magic:header].decode().split())
	assert(width == WIDTH and height == HEIGHT)
	# "On" for Playdate is black, so invert the pixels
	return zlib.compress(bytes(map(lambda x: x ^ 0b11111111, file[header + 1:])))

if __name__ == "__main__":
	frames = list(map(read_frame, sys.argv[2:]))
	sys.stdout.buffer.write("Playdate VID\x00\x00\x00\x00".encode())
	sys.stdout.buffer.write(struct.pack("<H", len(frames)))
	sys.stdout.buffer.write(struct.pack("<H", 0)) # Not sure what this is
	sys.stdout.buffer.write(struct.pack("<f", 30))
	sys.stdout.buffer.write(struct.pack("<H", WIDTH))
	sys.stdout.buffer.write(struct.pack("<H", HEIGHT))
	offset = 0
	for frame in frames:
		sys.stdout.buffer.write(struct.pack("<I", offset << 2 | 0x01))
		offset += len(frame)
	sys.stdout.buffer.write(struct.pack("<I", offset << 2 | 0x01))
	for frame in frames:
		sys.stdout.buffer.write(frame)
	sys.stdout.buffer.flush()
