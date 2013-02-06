#!/bin/sh

export LD_LIBRARY_PATH=../src/.libs/
export GI_TYPELIB_PATH=$GI_TYPELIB_PATH:../src/
export PYTHONPATH=../python/

$1
