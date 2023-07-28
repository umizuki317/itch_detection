# coding: utf-8
from http.server import HTTPServer
from http.server import BaseHTTPRequestHandler
import urllib.request
url = "your spread sheet url"

class class1(BaseHTTPRequestHandler):
    def do_GET(self):
        self.send_response(200)
        a = self.line()
        self.send_header("User-Agent","test1")
        self.end_headers()
        html = "hoge"
        self.wfile.write(html.encode())
    def line(self):
        urllib.request.urlopen(url)
        return 0

ip = "Local host ip"
port = "your port number"

server = HTTPServer((ip, port), class1)

server.serve_forever()
