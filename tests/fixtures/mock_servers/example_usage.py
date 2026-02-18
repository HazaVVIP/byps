#!/usr/bin/env python3
"""
Example usage of mock servers in tests.
Shows how to integrate these servers in automated testing.
"""
import subprocess
import time
import requests
import os


class MockServerManager:
    """Context manager for running mock servers during tests."""
    
    def __init__(self, server_script, port, wait_time=2):
        self.server_script = server_script
        self.port = port
        self.wait_time = wait_time
        self.process = None
    
    def __enter__(self):
        """Start the server."""
        script_path = os.path.join(
            os.path.dirname(__file__), 
            self.server_script
        )
        self.process = subprocess.Popen(
            ['python3', script_path],
            stdout=subprocess.PIPE,
            stderr=subprocess.PIPE
        )
        time.sleep(self.wait_time)
        return self
    
    def __exit__(self, exc_type, exc_val, exc_tb):
        """Stop the server."""
        if self.process:
            self.process.terminate()
            try:
                self.process.wait(timeout=5)
            except subprocess.TimeoutExpired:
                self.process.kill()


def test_basic_server_bypass():
    """Example: Test basic path bypass techniques."""
    with MockServerManager('basic_server.py', 8000):
        # Test that /admin is blocked
        response = requests.get('http://127.0.0.1:8000/admin')
        assert response.status_code == 403, "Expected /admin to be blocked"
        
        # Test trailing slash bypass
        response = requests.get('http://127.0.0.1:8000/admin/')
        assert response.status_code == 200, "Trailing slash bypass failed"
        
        # Test URL encoding bypass
        response = requests.get('http://127.0.0.1:8000/%61dmin')
        assert response.status_code == 200, "URL encoding bypass failed"
        
        print("✓ Basic server bypass tests passed")


def test_waf_detection():
    """Example: Test WAF detection and blocking."""
    with MockServerManager('waf_server.py', 8001):
        # Test normal request passes
        response = requests.get('http://127.0.0.1:8001/api/users')
        assert response.status_code == 200, "Normal request should pass"
        
        # Test XSS detection
        response = requests.get('http://127.0.0.1:8001/?q=<script>alert(1)</script>')
        assert response.status_code == 403, "XSS should be blocked"
        assert 'cloudflare' in response.headers.get('Server', '').lower()
        
        # Test SQLi detection
        response = requests.get('http://127.0.0.1:8001/?id=1 union select password')
        assert response.status_code == 403, "SQLi should be blocked"
        
        print("✓ WAF detection tests passed")


def test_header_bypass():
    """Example: Test header-based bypass techniques."""
    with MockServerManager('header_server.py', 8002):
        # Test that /admin is blocked without headers
        response = requests.get('http://127.0.0.1:8002/admin')
        assert response.status_code == 403, "Expected /admin to be blocked"
        
        # Test X-Forwarded-For bypass
        headers = {'X-Forwarded-For': '127.0.0.1'}
        response = requests.get('http://127.0.0.1:8002/admin', headers=headers)
        assert response.status_code == 200, "X-Forwarded-For bypass failed"
        
        # Test X-Original-URL bypass
        headers = {'X-Original-URL': '/allowed'}
        response = requests.get('http://127.0.0.1:8002/restricted', headers=headers)
        assert response.status_code == 200, "X-Original-URL bypass failed"
        
        # Test combined bypass
        headers = {
            'X-Forwarded-For': '127.0.0.1',
            'X-Original-URL': '/allowed'
        }
        response = requests.get('http://127.0.0.1:8002/combined', headers=headers)
        assert response.status_code == 200, "Combined bypass failed"
        
        print("✓ Header bypass tests passed")


if __name__ == '__main__':
    print("Running mock server examples...\n")
    
    test_basic_server_bypass()
    test_waf_detection()
    test_header_bypass()
    
    print("\nAll examples completed successfully! ✓")
