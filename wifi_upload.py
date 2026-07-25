#!/usr/bin/env python3
"""
Test script for WLAN upload of C64 programs
Usage: python3 wifi_upload.py <esp32_ip> [<start_address_hex>] <program_file>
If start_address is omitted, it's read from the PRG file header (first 2 bytes)
"""

import sys
import requests
import time

def upload_program(ip, start_address, program_file):
    # Read program file
    with open(program_file, 'rb') as f:
        data = f.read()
    
    # For .prg files, first 2 bytes are load address
    if len(data) >= 2:
        file_address = int.from_bytes(data[0:2], 'little')
        if start_address is None:
            start_address = file_address
        elif file_address != start_address:
            print(f"Warning: File load address (0x{file_address:04x}) != specified address (0x{start_address:04x})")
            print("Using file's load address...")
            start_address = file_address
        program_data = data[2:]
    else:
        if start_address is None:
            print("Error: File too small to contain a load address")
            sys.exit(1)
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
    if len(sys.argv) == 3:
        ip = sys.argv[1]
        start_address = None
        program_file = sys.argv[2]
    elif len(sys.argv) == 4:
        ip = sys.argv[1]
        start_address = int(sys.argv[2], 16)
        program_file = sys.argv[3]
    else:
        print("Usage: python3 wifi_upload.py <esp32_ip> [<start_address_hex>] <program_file>")
        print()
        print("  If start_address is omitted, it's read from the PRG file header (first 2 bytes)")
        print()
        print("Examples:")
        print("  python3 wifi_upload.py 192.168.1.100 0801 program.prg")
        print("  python3 wifi_upload.py 192.168.1.100 program.prg")
        sys.exit(1)
    
    upload_program(ip, start_address, program_file)

if __name__ == "__main__":
    main()
