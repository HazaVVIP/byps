#!/usr/bin/env python3
"""
WAF Server simulating Web Application Firewall behavior.
Tests pattern blocking and encoding bypasses similar to Cloudflare.
"""
import sys
import re
from http.server import HTTPServer, BaseHTTPRequestHandler
from urllib.parse import unquote, parse_qs


class WAFHandler(BaseHTTPRequestHandler):
    # Patterns that trigger WAF blocking
    BLOCKED_PATTERNS = [
        r'\.\.\/',  # Path traversal
        r'<script',  # XSS
        r'union\s+select',  # SQL injection
        r'<iframe',  # XSS
        r'javascript:',  # XSS
        r'onerror=',  # XSS
    ]
    
    def log_message(self, format, *args):
        sys.stderr.write(f"[WAF] {format % args}\n")

    def add_cloudflare_headers(self):
        """Add Cloudflare-like headers"""
        self.send_header('Server', 'cloudflare')
        self.send_header('CF-RAY', '1234567890abc-SJC')
        self.send_header('CF-Cache-Status', 'DYNAMIC')

    def check_waf(self, path, query_string=''):
        """Check if request should be blocked by WAF"""
        full_url = path
        if query_string:
            full_url = f"{path}?{query_string}"
        
        # Check decoded version for patterns
        decoded_url = unquote(full_url)
        
        for pattern in self.BLOCKED_PATTERNS:
            if re.search(pattern, decoded_url, re.IGNORECASE):
                return True, pattern
        
        return False, None

    def send_waf_block(self, pattern):
        """Send WAF block page"""
        self.send_response(403)
        self.send_header('Content-Type', 'text/html')
        self.add_cloudflare_headers()
        self.end_headers()
        
        html = f"""
        <html>
        <head><title>Access Denied</title></head>
        <body>
            <h1>Access Denied</h1>
            <p>Your request has been blocked by our Web Application Firewall.</p>
            <p>Reason: Suspicious pattern detected ({pattern})</p>
            <hr>
            <p><small>Cloudflare Ray ID: 1234567890abc-SJC</small></p>
        </body>
        </html>
        """
        self.wfile.write(html.encode())

    def do_GET(self):
        path = self.path.split('?')[0]
        query_string = self.path.split('?')[1] if '?' in self.path else ''
        
        # Check WAF
        blocked, pattern = self.check_waf(path, query_string)
        if blocked:
            self.send_waf_block(pattern)
            return
        
        # If not blocked, allow the request
        self.send_response(200)
        self.send_header('Content-Type', 'text/html')
        self.add_cloudflare_headers()
        self.end_headers()
        
        response = f"""
        <html>
        <body>
            <h1>Request Allowed</h1>
            <p>Path: {path}</p>
            <p>Query: {query_string}</p>
            <p>This request passed the WAF checks</p>
        </body>
        </html>
        """
        self.wfile.write(response.encode())

    def do_POST(self):
        content_length = int(self.headers.get('Content-Length', 0))
        post_data = self.rfile.read(content_length).decode('utf-8', errors='ignore')
        
        path = self.path.split('?')[0]
        
        # Check WAF on path and POST data
        blocked, pattern = self.check_waf(path, post_data)
        if blocked:
            self.send_waf_block(pattern)
            return
        
        # If not blocked, allow the request
        self.send_response(200)
        self.send_header('Content-Type', 'text/html')
        self.add_cloudflare_headers()
        self.end_headers()
        
        response = f"""
        <html>
        <body>
            <h1>POST Request Allowed</h1>
            <p>Path: {path}</p>
            <p>Data length: {content_length}</p>
            <p>This request passed the WAF checks</p>
        </body>
        </html>
        """
        self.wfile.write(response.encode())


def run_server(host='127.0.0.1', port=8001):
    server_address = (host, port)
    httpd = HTTPServer(server_address, WAFHandler)
    print(f"WAF Server running on http://{host}:{port}")
    print("Blocked patterns:")
    for pattern in WAFHandler.BLOCKED_PATTERNS:
        print(f"  - {pattern}")
    print("\nTest cases:")
    print(f"  - http://{host}:{port}/../etc/passwd      -> 403 (path traversal)")
    print(f"  - http://{host}:{port}/?q=<script>        -> 403 (XSS)")
    print(f"  - http://{host}:{port}/?q=union+select    -> 403 (SQLi)")
    print(f"  - http://{host}:{port}/?q=%3Cscript%3E    -> 403 (encoded XSS)")
    print(f"  - http://{host}:{port}/normal             -> 200 OK")
    try:
        httpd.serve_forever()
    except KeyboardInterrupt:
        print("\nShutting down WAF Server...")
        httpd.shutdown()


if __name__ == '__main__':
    run_server()
