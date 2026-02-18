#!/usr/bin/env python3
"""
Basic HTTP Server for testing path bypass techniques.
Tests normalization, encoding, and trailing slash behaviors.
"""
import sys
from http.server import HTTPServer, BaseHTTPRequestHandler
from urllib.parse import unquote


class BasicHandler(BaseHTTPRequestHandler):
    def log_message(self, format, *args):
        sys.stderr.write(f"[BASIC] {format % args}\n")

    def do_GET(self):
        path = self.path.split('?')[0]
        
        # Block /admin (no trailing slash)
        if path == '/admin':
            self.send_response(403)
            self.send_header('Content-Type', 'text/html')
            self.end_headers()
            self.wfile.write(b'<html><body><h1>403 Forbidden</h1><p>Access to /admin is denied</p></body></html>')
            return
        
        # Allow /admin/ (trailing slash bypass)
        if path == '/admin/':
            self.send_response(200)
            self.send_header('Content-Type', 'text/html')
            self.end_headers()
            self.wfile.write(b'<html><body><h1>Admin Panel</h1><p>Bypassed via trailing slash</p></body></html>')
            return
        
        # Allow encoded version /%61dmin (URL encoding bypass)
        if path == '/%61dmin':
            self.send_response(200)
            self.send_header('Content-Type', 'text/html')
            self.end_headers()
            self.wfile.write(b'<html><body><h1>Admin Panel</h1><p>Bypassed via URL encoding</p></body></html>')
            return
        
        # Allow all other paths
        self.send_response(200)
        self.send_header('Content-Type', 'text/html')
        self.end_headers()
        response = f'<html><body><h1>200 OK</h1><p>Path: {path}</p></body></html>'
        self.wfile.write(response.encode())

    def do_POST(self):
        self.do_GET()


def run_server(host='127.0.0.1', port=8000):
    server_address = (host, port)
    httpd = HTTPServer(server_address, BasicHandler)
    print(f"Basic Server running on http://{host}:{port}")
    print("Test cases:")
    print(f"  - http://{host}:{port}/admin      -> 403 Forbidden")
    print(f"  - http://{host}:{port}/admin/     -> 200 OK (trailing slash bypass)")
    print(f"  - http://{host}:{port}/%61dmin    -> 200 OK (encoding bypass)")
    print(f"  - http://{host}:{port}/other      -> 200 OK")
    try:
        httpd.serve_forever()
    except KeyboardInterrupt:
        print("\nShutting down Basic Server...")
        httpd.shutdown()


if __name__ == '__main__':
    run_server()
