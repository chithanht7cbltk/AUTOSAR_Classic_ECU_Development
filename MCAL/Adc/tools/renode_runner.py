import os
import sys
import subprocess

def run_renode(script_path):
    print(f"Starting Renode with script: {script_path}")
    
    # Path to renode executable (assuming it's in PATH)
    renode_cmd = ["renode", "--disable-x11", "--console", script_path]
    
    try:
        process = subprocess.Popen(renode_cmd, stdout=subprocess.PIPE, stderr=subprocess.PIPE, text=True)
        
        while True:
            output = process.stdout.readline()
            if output == '' and process.poll() is not None:
                break
            if output:
                print(output.strip())
                
        rc = process.poll()
        if rc != 0:
            print(f"Renode exited with code {rc}")
            return False
            
        return True
    except FileNotFoundError:
        print("Error: Renode not found in PATH. Please install Renode.")
        return False

if __name__ == "__main__":
    if len(sys.argv) < 2:
        print("Usage: python renode_runner.py <resc_script>")
        sys.exit(1)
        
    script = sys.argv[1]
    if run_renode(script):
        print("Simulation completed.")
    else:
        print("Simulation failed.")
        sys.exit(1)
