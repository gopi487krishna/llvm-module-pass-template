clang  -S -emit-llvm -fno-discard-value-names   -flegacy-pass-manager -Xclang -load $(passlocation) -Xclang mp_test.c 
