#!/usr/bin/env python3
"""
Basic HTTP server for testing path bypass techniques.
This server blocks /admin but allows various bypass variations.
"""

from http.server import HTTPServer, BaseHTTPRequestHandler
import sys

class BypassTestHandler(BaseHTTPRequestHandler):
    def log_message(self, format, *args):
        """Log requests to stdout"""
        sys.stdout.write("%s - - [%s] %s\n" %
                         (self.address_string(),
                          self.log_date_time_string(),
                          format % args))
        sys.stdout.flush()
    
    def do_GET(self):
        """Handle GET requests"""
        path = self.path
        
        # Block exact /admin path
        if path == '/admin':
            self.send_response(403)
            self.send_header('Content-type', 'text/html')
            self.send_header('Server', 'TestServer/1.0')
            self.end_headers()
            self.wfile.write(b'<html><body><h1>403 Forbidden</h1><p>Access Denied</p></body></html>')
            return
        
        # Allow trailing slash bypass
        if path == '/admin/':
            self.send_response(200)
            self.send_header('Content-type', 'text/html')
            self.send_header('Server', 'TestServer/1.0')
            self.end_headers()
            self.wfile.write(b'<html><body><h1>Admin Panel</h1><p>Bypass successful: trailing slash</p></body></html>')
            return
        
        # Allow case variation bypass
        if path.lower() == '/admin' and path != '/admin':
            self.send_response(200)
            self.send_header('Content-type', 'text/html')
            self.send_header('Server', 'TestServer/1.0')
            self.end_headers()
            self.wfile.write(b'<html><body><h1>Admin Panel</h1><p>Bypass successful: case variation</p></body></html>')
            return
        
        # Allow dot segment bypass
        if '/./' in path and 'admin' in path:
            self.send_response(200)
            self.send_header('Content-type', 'text/html')
            self.send_header('Server', 'TestServer/1.0')
            self.end_headers()
            self.wfile.write(b'<html><body><h1>Admin Panel</h1><p>Bypass successful: dot segment</p></body></html>')
            return
        
        # Default response
        self.send_response(200)
        self.send_header('Content-type', 'text/html')
        self.send_header('Server', 'TestServer/1.0')
        self.end_headers()
        self.wfile.write(b'<html><body><h1>Welcome</h1><p>Public page</p></body></html>')
    
    def do_POST(self):
        """Handle POST requests"""
        self.do_GET()
    
    def do_HEAD(self):
        """Handle HEAD requests"""
        self.send_response(200)
        self.send_header('Content-type', 'text/html')
        self.send_header('Server', 'TestServer/1.0')
        self.end_headers()

def run(host='127.0.0.1', port=8000):
    server_address = (host, port)
    httpd = HTTPServer(server_address, BypassTestHandler)
    print(f'Starting basic test server on http://{host}:{port}')
    print(f'Test URLs:')
    print(f'  - http://{host}:{port}/admin (403 - blocked)')
    print(f'  - http://{host}:{port}/admin/ (200 - bypass with trailing slash)')
    print(f'  - http://{host}:{port}/ADMIN (200 - bypass with case variation)')
    print(f'  - http://{host}:{port}/./admin (200 - bypass with dot segment)')
    print(f'Press Ctrl+C to stop')
    try:
        httpd.serve_forever()
    except KeyboardInterrupt:
        print('\nServer stopped')
        httpd.shutdown()

if __name__ == '__main__':
    port = 8000
    if len(sys.argv) > 1:
        port = int(sys.argv[1])
    run(port=port)
