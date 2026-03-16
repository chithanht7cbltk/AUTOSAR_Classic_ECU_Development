import os
import sys

def run_tests():
    print("Running ADC Driver Tests...")
    
    # Normally this would invoke renode, compile the ELF, and check the output
    # For this example, we mock a successful run if standard paths exist
    
    if not os.path.exists("examples/Makefile"):
        print("Error: Examples Makefile not found")
        sys.exit(1)
        
    print("Test: Compiling main_dma...")
    compile_cmd = "cd examples && make TARGET=main_dma clean all"
    if os.system(compile_cmd) != 0:
        print("FAIL: Compilation main_dma failed")
        sys.exit(1)
        
    print("Test: main_dma passed!")
    
    print("Test: Compiling main_irq...")
    compile_cmd = "cd examples && make TARGET=main_irq clean all"
    if os.system(compile_cmd) != 0:
        print("FAIL: Compilation main_irq failed")
        sys.exit(1)
        
    print("Test: main_irq passed!")

    print("Test: Compiling main_combined...")
    compile_cmd = "cd examples && make TARGET=main_combined clean all"
    if os.system(compile_cmd) != 0:
        print("FAIL: Compilation main_combined failed")
        sys.exit(1)
        
    print("Test: main_combined passed!")
    
    print("\nAll compilation tests passed successfully!")

if __name__ == "__main__":
    run_tests()
