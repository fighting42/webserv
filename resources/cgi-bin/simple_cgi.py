#!/usr/bin/env python3
import os
import sys

print("SIMPLE CGI\n")
print("HTTP METHOD: {}".format(os.environ['REQUEST_METHOD']))

if os.environ['REQUEST_METHOD'] == 'GET':
    query_string = os.environ['QUERY_STRING']
    print("QUERY STRING: {}".format(query_string))

if os.environ['REQUEST_METHOD'] == 'POST':
    content_length = int(os.environ.get('CONTENT_LENGTH', 0))
    post_data = sys.stdin.read(content_length)
    print("DATA: {}".format(post_data))
