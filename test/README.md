Test Infra

1. bad:
Trying to allocate more memory than the default.
Trying to free an illegal memory address.
Should not crash.

2. c_test:
Trying to allocate some random length memory block, then resize them.
Magic numbers are written to the region to ensure they are not polluted.
The free ensures the reuse of memory.

3. py_test:
Python script using numpy that calls malloc.
Inherited from https://github.com/misko/bigmaac

Build the test:
make all

Run the test:
LD_PRELOAD=$PATH_TO_YOUR_LIB xxx.exe
LD_PRELOAD=$PATH_TO_YOUR_LIB python3 xxx.py