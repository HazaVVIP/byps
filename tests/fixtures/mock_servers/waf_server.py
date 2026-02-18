#!/usr/bin/env python3
"""
Mock WAF server for testing WAF detection and bypass techniques.
Simulates a simple WAF that blocks certain patterns.
"""

from http.server import HTTPServer, BaseHTTPRequestHandler
import sys
import re

class MockWAFHandler(BaseHTTPRequestHandler):
    # WAF rules - patterns to block
    BLOCKED_PATTERNS = [
        r'/admin',
        r'\.\./',
        r'<script>',
        r'union\s+select',
        r'drop\s+table',
        r'exec\(',
    ]
    
    def log_message(self, format, *args):
        sys.stdout.write("%s - - [%s] %s\n" %
                         (self.address_string(),
                          self.log_date_time_string(),
                          format % args))
        sys.stdout.flush()
    
    def check_waf_rules(self, path):
        """Check if path matches any WAF blocking patterns"""
        for pattern in self.BLOCKED_PATTERNS:
            if re.search(pattern, path, re.IGNORECASE):
                return True, pattern
        return False, None
    
    def send_waf_block_response(self, pattern):
        """Send WAF block response"""
        self.send_response(403)
        self.send_header('Content-type', 'text/html')
        self.send_header('Server', 'MockWAF/1.0')
        self.send_header('X-WAF-Block', 'true')
        self.send_header('X-WAF-Rule', pattern)
        self.end_headers()
        html = f'''
        <html>
        <head><title>403 Forbidden</title></head>
        <body>
        <h1>Access Denied</h1>
        <p>Your request was blocked by the Web Application Firewall.</p>
        <p>Rule triggered: {pattern}</p>
        <hr>
        <p><small>MockWAF/1.0</small></p>
        </body>
        </html>
        '''
        self.wfile.write(html.encode())
    
    def send_allowed_response(self, path):
        """Send allowed response"""
        self.send_response(200)
        self.send_header('Content-type', 'text/html')
        self.send_header('Server', 'MockWAF/1.0')
        self.end_headers()
        html = f'''
        <html>
        <head><title>Page Accessed</title></head>
        <body>
        <h1>Access Granted</h1>
        <p>Path accessed: {path}</p>
        <p>Request passed WAF inspection.</p>
        </body>
        </html>
        '''
        self.wfile.write(html.encode())
    
    def do_GET(self):
        blocked, pattern = self.check_waf_rules(self.path)
        
        if blocked:
            self.send_waf_block_response(pattern)
        else:
            self.send_allowed_response(self.path)
    
    def do_POST(self):
        self.do_GET()
    
    def do_HEAD(self):
        blocked, pattern = self.check_waf_rules(self.path)
        
        if blocked:
            self.send_response(403)
            self.send_header('Server', 'MockWAF/1.0')
            self.send_header('X-WAF-Block', 'true')
            self.end_headers()
        else:
            self.send_response(200)
            self.send_header('Server', 'MockWAF/1.0')
            self.end_headers()

def run(host='127.0.0.1', port=8002):
    server_address = (host, port)
    httpd = HTTPServer(server_address, MockWAFHandler)
    print(f'Starting Mock WAF server on http://{host}:{port}')
    print(f'WAF Rules:')
    for pattern in MockWAFHandler.BLOCKED_PATTERNS:
        print(f'  - Block pattern: {pattern}')
    print(f'\nTest URLs:')
    print(f'  - http://{host}:{port}/admin (blocked)')
    print(f'  - http://{host}:{port}/public (allowed)')
    print(f'Press Ctrl+C to stop')
    try:
        httpd.serve_forever()
    except KeyboardInterrupt:
        print('\nServer stopped')
        httpd.shutdown()

if __name__ == '__main__':
    port = 8002
    if len(sys.argv) > 1:
        port = int(sys.argv[1])
    run(port=port)
