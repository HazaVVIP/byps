#!/usr/bin/env python3
"""
Quick test script for all mock servers.
Starts each server briefly and tests basic functionality.
"""
import subprocess
import time
import sys
import signal
import os

def test_basic_server():
    """Test basic_server.py"""
    print("Testing basic_server.py...")
    proc = subprocess.Popen(
        [sys.executable, 'basic_server.py'],
        stdout=subprocess.PIPE,
        stderr=subprocess.PIPE,
        cwd=os.path.dirname(__file__)
    )
    
    time.sleep(2)
    
    try:
        # Test with curl
        result = subprocess.run(
            ['curl', '-s', '-o', '/dev/null', '-w', '%{http_code}', 'http://127.0.0.1:8000/admin'],
            capture_output=True,
            text=True,
            timeout=5
        )
        if result.stdout == '403':
            print("  ✓ /admin returns 403")
        else:
            print(f"  ✗ /admin returned {result.stdout}, expected 403")
            return False
        
        result = subprocess.run(
            ['curl', '-s', '-o', '/dev/null', '-w', '%{http_code}', 'http://127.0.0.1:8000/admin/'],
            capture_output=True,
            text=True,
            timeout=5
        )
        if result.stdout == '200':
            print("  ✓ /admin/ returns 200")
        else:
            print(f"  ✗ /admin/ returned {result.stdout}, expected 200")
            return False
        
        result = subprocess.run(
            ['curl', '-s', '-o', '/dev/null', '-w', '%{http_code}', 'http://127.0.0.1:8000/%61dmin'],
            capture_output=True,
            text=True,
            timeout=5
        )
        if result.stdout == '200':
            print("  ✓ /%61dmin returns 200")
        else:
            print(f"  ✗ /%61dmin returned {result.stdout}, expected 200")
            return False
        
        print("  ✓ All tests passed\n")
        return True
    finally:
        proc.terminate()
        proc.wait(timeout=5)


def test_waf_server():
    """Test waf_server.py"""
    print("Testing waf_server.py...")
    proc = subprocess.Popen(
        [sys.executable, 'waf_server.py'],
        stdout=subprocess.PIPE,
        stderr=subprocess.PIPE,
        cwd=os.path.dirname(__file__)
    )
    
    time.sleep(2)
    
    try:
        # Test normal request
        result = subprocess.run(
            ['curl', '-s', '-o', '/dev/null', '-w', '%{http_code}', 'http://127.0.0.1:8001/normal'],
            capture_output=True,
            text=True,
            timeout=5
        )
        if result.stdout == '200':
            print("  ✓ /normal returns 200")
        else:
            print(f"  ✗ /normal returned {result.stdout}, expected 200")
            return False
        
        # Test blocked pattern
        result = subprocess.run(
            ['curl', '-s', '-o', '/dev/null', '-w', '%{http_code}', 'http://127.0.0.1:8001/?path=../etc/passwd'],
            capture_output=True,
            text=True,
            timeout=5
        )
        if result.stdout == '403':
            print("  ✓ ?path=../etc/passwd returns 403")
        else:
            print(f"  ✗ ?path=../etc/passwd returned {result.stdout}, expected 403")
            return False
        
        print("  ✓ All tests passed\n")
        return True
    finally:
        proc.terminate()
        proc.wait(timeout=5)


def test_header_server():
    """Test header_server.py"""
    print("Testing header_server.py...")
    proc = subprocess.Popen(
        [sys.executable, 'header_server.py'],
        stdout=subprocess.PIPE,
        stderr=subprocess.PIPE,
        cwd=os.path.dirname(__file__)
    )
    
    time.sleep(2)
    
    try:
        # Test /admin without header
        result = subprocess.run(
            ['curl', '-s', '-o', '/dev/null', '-w', '%{http_code}', 'http://127.0.0.1:8002/admin'],
            capture_output=True,
            text=True,
            timeout=5
        )
        if result.stdout == '403':
            print("  ✓ /admin without header returns 403")
        else:
            print(f"  ✗ /admin without header returned {result.stdout}, expected 403")
            return False
        
        # Test /admin with header
        result = subprocess.run(
            ['curl', '-s', '-o', '/dev/null', '-w', '%{http_code}', '-H', 'X-Forwarded-For: 127.0.0.1', 'http://127.0.0.1:8002/admin'],
            capture_output=True,
            text=True,
            timeout=5
        )
        if result.stdout == '200':
            print("  ✓ /admin with X-Forwarded-For returns 200")
        else:
            print(f"  ✗ /admin with X-Forwarded-For returned {result.stdout}, expected 200")
            return False
        
        print("  ✓ All tests passed\n")
        return True
    finally:
        proc.terminate()
        proc.wait(timeout=5)


if __name__ == '__main__':
    os.chdir(os.path.dirname(__file__))
    
    print("=" * 60)
    print("Mock Server Tests")
    print("=" * 60 + "\n")
    
    results = []
    results.append(("Basic Server", test_basic_server()))
    results.append(("WAF Server", test_waf_server()))
    results.append(("Header Server", test_header_server()))
    
    print("=" * 60)
    print("Summary")
    print("=" * 60)
    
    all_passed = True
    for name, passed in results:
        status = "✓ PASS" if passed else "✗ FAIL"
        print(f"{name:20} {status}")
        if not passed:
            all_passed = False
    
    print()
    
    if all_passed:
        print("All tests passed! ✓")
        sys.exit(0)
    else:
        print("Some tests failed! ✗")
        sys.exit(1)
