#!/usr/bin/env python3
"""
Header-based bypass test server.
Tests X-Forwarded-For, X-Original-URL, and other header bypass techniques.
"""

from http.server import HTTPServer, BaseHTTPRequestHandler
import sys

class HeaderBypassHandler(BaseHTTPRequestHandler):
    def log_message(self, format, *args):
        sys.stdout.write("%s - - [%s] %s\n" %
                         (self.address_string(),
                          self.log_date_time_string(),
                          format % args))
        sys.stdout.flush()
    
    def do_GET(self):
        path = self.path
        
        # Check for X-Forwarded-For bypass
        xff = self.headers.get('X-Forwarded-For', '')
        if xff in ['127.0.0.1', 'localhost', '::1']:
            self.send_response(200)
            self.send_header('Content-type', 'text/html')
            self.send_header('Server', 'HeaderTestServer/1.0')
            self.end_headers()
            self.wfile.write(f'<html><body><h1>Access Granted</h1><p>Bypass via X-Forwarded-For: {xff}</p></body></html>'.encode())
            return
        
        # Check for X-Original-URL bypass
        original_url = self.headers.get('X-Original-URL', '')
        if original_url and original_url != path:
            self.send_response(200)
            self.send_header('Content-type', 'text/html')
            self.send_header('Server', 'HeaderTestServer/1.0')
            self.end_headers()
            self.wfile.write(f'<html><body><h1>Access Granted</h1><p>Bypass via X-Original-URL: {original_url}</p></body></html>'.encode())
            return
        
        # Check for X-Rewrite-URL bypass
        rewrite_url = self.headers.get('X-Rewrite-URL', '')
        if rewrite_url:
            self.send_response(200)
            self.send_header('Content-type', 'text/html')
            self.send_header('Server', 'HeaderTestServer/1.0')
            self.end_headers()
            self.wfile.write(f'<html><body><h1>Access Granted</h1><p>Bypass via X-Rewrite-URL: {rewrite_url}</p></body></html>'.encode())
            return
        
        # Check for Host header bypass
        host = self.headers.get('Host', '')
        if host in ['localhost', '127.0.0.1', '::1']:
            self.send_response(200)
            self.send_header('Content-type', 'text/html')
            self.send_header('Server', 'HeaderTestServer/1.0')
            self.end_headers()
            self.wfile.write(f'<html><body><h1>Access Granted</h1><p>Bypass via Host header: {host}</p></body></html>'.encode())
            return
        
        # Default: block access
        self.send_response(403)
        self.send_header('Content-type', 'text/html')
        self.send_header('Server', 'HeaderTestServer/1.0')
        self.end_headers()
        self.wfile.write(b'<html><body><h1>403 Forbidden</h1><p>Access Denied</p></body></html>')

    def do_POST(self):
        self.do_GET()

def run(host='127.0.0.1', port=8001):
    server_address = (host, port)
    httpd = HTTPServer(server_address, HeaderBypassHandler)
    print(f'Starting header bypass test server on http://{host}:{port}')
    print(f'Test with headers:')
    print(f'  - X-Forwarded-For: 127.0.0.1')
    print(f'  - X-Original-URL: /admin')
    print(f'  - X-Rewrite-URL: /admin')
    print(f'  - Host: localhost')
    print(f'Press Ctrl+C to stop')
    try:
        httpd.serve_forever()
    except KeyboardInterrupt:
        print('\nServer stopped')
        httpd.shutdown()

if __name__ == '__main__':
    port = 8001
    if len(sys.argv) > 1:
        port = int(sys.argv[1])
    run(port=port)
