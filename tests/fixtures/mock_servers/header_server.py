#!/usr/bin/env python3
"""
Header-sensitive HTTP Server for testing header-based bypasses.
Tests X-Forwarded-For, X-Original-URL, and other header forgery techniques.
"""
import sys
from http.server import HTTPServer, BaseHTTPRequestHandler


class HeaderHandler(BaseHTTPRequestHandler):
    def log_message(self, format, *args):
        sys.stderr.write(f"[HEADER] {format % args}\n")

    def check_ip_whitelist(self):
        """Check if X-Forwarded-For header contains whitelisted IP"""
        xff = self.headers.get('X-Forwarded-For', '')
        x_real_ip = self.headers.get('X-Real-IP', '')
        
        # Check if any whitelisted IP is present
        whitelisted = ['127.0.0.1', 'localhost', '::1']
        for ip in whitelisted:
            if ip in xff or ip in x_real_ip:
                return True
        return False

    def check_url_override(self):
        """Check URL override headers"""
        x_original_url = self.headers.get('X-Original-URL', '')
        x_rewrite_url = self.headers.get('X-Rewrite-URL', '')
        
        # Allow if override points to /allowed or /public
        allowed_paths = ['/allowed', '/public']
        for path in allowed_paths:
            if path in x_original_url or path in x_rewrite_url:
                return True
        return False

    def do_GET(self):
        path = self.path.split('?')[0]
        
        # /admin endpoint - requires IP whitelist bypass
        if path == '/admin':
            if self.check_ip_whitelist():
                self.send_response(200)
                self.send_header('Content-Type', 'text/html')
                self.end_headers()
                response = """
                <html>
                <body>
                    <h1>Admin Panel</h1>
                    <p>Access granted via IP whitelist bypass</p>
                    <p>X-Forwarded-For header accepted</p>
                </body>
                </html>
                """
                self.wfile.write(response.encode())
                return
            else:
                self.send_response(403)
                self.send_header('Content-Type', 'text/html')
                self.end_headers()
                response = """
                <html>
                <body>
                    <h1>403 Forbidden</h1>
                    <p>Access denied: IP not whitelisted</p>
                    <p>Try adding X-Forwarded-For: 127.0.0.1</p>
                </body>
                </html>
                """
                self.wfile.write(response.encode())
                return
        
        # /restricted endpoint - requires URL override bypass
        if path == '/restricted':
            if self.check_url_override():
                self.send_response(200)
                self.send_header('Content-Type', 'text/html')
                self.end_headers()
                response = """
                <html>
                <body>
                    <h1>Restricted Area</h1>
                    <p>Access granted via URL override bypass</p>
                    <p>X-Original-URL header accepted</p>
                </body>
                </html>
                """
                self.wfile.write(response.encode())
                return
            else:
                self.send_response(403)
                self.send_header('Content-Type', 'text/html')
                self.end_headers()
                response = """
                <html>
                <body>
                    <h1>403 Forbidden</h1>
                    <p>Access denied: Restricted resource</p>
                    <p>Try adding X-Original-URL: /allowed</p>
                </body>
                </html>
                """
                self.wfile.write(response.encode())
                return
        
        # /combined endpoint - requires both bypasses
        if path == '/combined':
            ip_check = self.check_ip_whitelist()
            url_check = self.check_url_override()
            
            if ip_check and url_check:
                self.send_response(200)
                self.send_header('Content-Type', 'text/html')
                self.end_headers()
                response = """
                <html>
                <body>
                    <h1>Combined Protection Bypassed</h1>
                    <p>Both IP whitelist and URL override checks passed</p>
                </body>
                </html>
                """
                self.wfile.write(response.encode())
                return
            else:
                self.send_response(403)
                self.send_header('Content-Type', 'text/html')
                self.end_headers()
                missing = []
                if not ip_check:
                    missing.append("IP whitelist (X-Forwarded-For: 127.0.0.1)")
                if not url_check:
                    missing.append("URL override (X-Original-URL: /allowed)")
                
                response = f"""
                <html>
                <body>
                    <h1>403 Forbidden</h1>
                    <p>Access denied. Missing: {', '.join(missing)}</p>
                </body>
                </html>
                """
                self.wfile.write(response.encode())
                return
        
        # Echo headers for debugging
        if path == '/headers':
            self.send_response(200)
            self.send_header('Content-Type', 'text/html')
            self.end_headers()
            
            headers_html = '<br>'.join([f"{k}: {v}" for k, v in self.headers.items()])
            response = f"""
            <html>
            <body>
                <h1>Request Headers</h1>
                <p>{headers_html}</p>
            </body>
            </html>
            """
            self.wfile.write(response.encode())
            return
        
        # Default response
        self.send_response(200)
        self.send_header('Content-Type', 'text/html')
        self.end_headers()
        response = f"""
        <html>
        <body>
            <h1>200 OK</h1>
            <p>Path: {path}</p>
            <p>Public resource</p>
        </body>
        </html>
        """
        self.wfile.write(response.encode())

    def do_POST(self):
        self.do_GET()


def run_server(host='127.0.0.1', port=8002):
    server_address = (host, port)
    httpd = HTTPServer(server_address, HeaderHandler)
    print(f"Header Server running on http://{host}:{port}")
    print("\nTest cases:")
    print(f"  - http://{host}:{port}/admin")
    print(f"    Without headers: 403")
    print(f"    With X-Forwarded-For: 127.0.0.1: 200")
    print(f"  - http://{host}:{port}/restricted")
    print(f"    Without headers: 403")
    print(f"    With X-Original-URL: /allowed: 200")
    print(f"  - http://{host}:{port}/combined")
    print(f"    Requires both headers: 200")
    print(f"  - http://{host}:{port}/headers")
    print(f"    Shows all request headers")
    try:
        httpd.serve_forever()
    except KeyboardInterrupt:
        print("\nShutting down Header Server...")
        httpd.shutdown()


if __name__ == '__main__':
    run_server()
