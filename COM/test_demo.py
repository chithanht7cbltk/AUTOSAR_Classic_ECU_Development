import subprocess, time, os

os.system("pkill -9 -f renode > /dev/null 2>&1")

print("Starting run_demo.py...")
proc = subprocess.Popen(["python3", "run_demo.py"], text=True)

time.sleep(5)
print("Connecting GDB to resume execution (simulate F5)...")
subprocess.run(["arm-none-eabi-gdb", "node_tx.elf", "--batch", "-ex", "target remote :3333", "-ex", "c &", "-ex", "detach", "-ex", "quit"], stdout=subprocess.DEVNULL, stderr=subprocess.DEVNULL)

time.sleep(5)
print("Triggering TX (simulate setting watch variable)...")
subprocess.run(["arm-none-eabi-gdb", "node_tx.elf", "--batch", "-ex", "target remote :3333", "-ex", "set trigger_tx=1", "-ex", "c &", "-ex", "detach", "-ex", "quit"], stdout=subprocess.DEVNULL, stderr=subprocess.DEVNULL)

time.sleep(5)
proc.terminate()
