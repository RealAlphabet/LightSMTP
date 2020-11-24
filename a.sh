#!/bin/sh

gcc testbdd.c $(pkg-config --libs --cflags libmongoc-1.0)
