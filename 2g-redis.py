#!/usr/bin/env python

import hashlib
import os
import sys

import redis

def hashpath(path):
    sha1 = hashlib.sha1()
    sha1.update(path)
    return sha1.hexdigest()

def index(path):
    r = redis.StrictRedis(host='localhost', port=6379, db=0)
    linenum = 0
    h = hashpath(path)
    f = open(path, "rb")

    for line in f.readlines():
        for pos in range(0, len(line) - 1):
            key = "2g:index:" + h + ":" + line[pos:pos+2]
            r.zadd(key, linenum, linenum)
        linenum += 1

def main():
    p = os.path.normpath(sys.argv[1])
    assert os.path.isfile(p), "file not found: " + p
    index(p)

# main()

index("./2g-redis.py")

# (table -> column -> key) -> [value]
