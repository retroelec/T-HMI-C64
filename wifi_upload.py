#!/usr/bin/env python3
"""
Test script for WLAN upload of C64 programs
Usage: python3 wifi_upload.py <esp32_ip> <start_address_hex> <program_file>
Example: python3 wifi_upload.py 192.168.1.100 0801 program.prg
"""

import sys
import requests
import time

def upload_program(ip, start_address, program_file):
    # Read program file
    with open(program_file, 'rb') as f:
        data = f.read()
    
    # For .prg files, first 2 bytes are load address
    # Remove them if they exist
    if len(data) >= 2:
        actual_start = int.from_bytes(data[0:2], 'little')
        if actual_start != start_address:
            print(f"Warning: File load address (0x{actual_start:04x}) doesn't match specified address (0x{start_address:04x})")
            print("Using file's load address...")
            start_address = actual_start
            program_data = data[2:]  # Skip load address bytes
        else:
            program_data = data[2:]
    else:
        program_data = data
    
    # Convert to hex string
    hex_data = program_data.hex()
    
    # Prepare POST request
    url = f"http://{ip}/upload_c64_program"
    params = {
        'start': f"{start_address:04x}",
        'data': hex_data
    }
    
    print(f"Uploading {len(program_data)} bytes to address 0x{start_address:04x}...")
    print(f"URL: {url}")
    
    try:
        response = requests.post(url, data=params, timeout=30)
        print(f"Response: {response.status_code}")
        print(f"Message: {response.text}")
    except requests.exceptions.RequestException as e:
        print(f"Error: {e}")

def main():
    if len(sys.argv) != 4:
        print("Usage: python3 wifi_upload.py <esp32_ip> <start_address_hex> <program_file>")
        print("Example: python3 wifi_upload.py 192.168.1.100 0801 program.prg")
        sys.exit(1)
    
    ip = sys.argv[1]
    start_address = int(sys.argv[2], 16)
    program_file = sys.argv[3]
    
    upload_program(ip, start_address, program_file)

if __name__ == "__main__":
    main()
