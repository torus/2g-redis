#!/usr/bin/env python

import hashlib

import redis

def hashpath(path):
    sha1 = hashlib.sha1()
    sha1.update(path)
    return sha1.hexdigest()

def lookup(path, phrase):
    r = redis.StrictRedis(host='localhost', port=6379, db=0)
    args = []
    score = {}
    for i in range(0, len(phrase.rstrip()) - 1):
        h = hashpath(path)
        seg = phrase[i:i+2]
        key = "2g:index:" + h + ":" + seg
        print key

        res = r.zrange(key, 0, -1)
        for i in res:
            score[i] = (score.has_key(i) and score[i] or 0) + 1

    intermed = sorted(score.items(), key=lambda x: x[1], reverse=True)
    print intermed
    lines = [x[0] for x in intermed[0:5]]
    show_lines(path, sorted(lines))

def show_lines(path, lines):
    infile = open(path, "rb")
    linecount = 0
    index = 0
    for line in infile:
        if linecount == int(lines[index]):
            print line.rstrip()
            index += 1
            if index >= len(lines):
                break
        linecount += 1

lookup("./2g-redis.py", "normpath")
