# LLVM Module Pass Template

Simple LLVM Module Pass Template  

# Dependencies

- cmake
- ninja
- llvm

# Building Module Pass Template

Run the script given below to build the LLVM module pass

```bash
build-modulepass.sh
```

Supply clean as the argument to remove the build artifacts


```bash
build-modulepass.sh clean
```
# Running the pass
    $ clang -flegacy-pass-manager -Xclang -load -Xclang $(passloc).* something.c
