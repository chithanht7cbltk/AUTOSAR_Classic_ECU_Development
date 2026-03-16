import os
import sys
import subprocess

def run_openocd(binary_path):
    print(f"Flashing {binary_path} using OpenOCD...")
    
    # Example config for STM32F103 (BluePill) with ST-Link V2
    openocd_cmd = [
        "openocd",
        "-f", "interface/stlink.cfg",
        "-f", "target/stm32f1x.cfg",
        "-c", f"program {binary_path} verify reset exit"
    ]
    
    try:
        process = subprocess.run(openocd_cmd, capture_output=True, text=True)
        
        print(process.stdout)
        if process.returncode != 0:
            print("Error flashing:")
            print(process.stderr)
            return False
            
        print("Flash successful!")
        return True
    except FileNotFoundError:
        print("Error: OpenOCD not found in PATH.")
        return False

if __name__ == "__main__":
    if len(sys.argv) < 2:
        print("Usage: python openocd_flash.py <binary_file.elf>")
        sys.exit(1)
        
    binary = sys.argv[1]
    if run_openocd(binary):
        sys.exit(0)
    else:
        sys.exit(1)
