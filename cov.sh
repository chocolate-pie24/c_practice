#!/bin/bash

LLVM_PROFILE_FILE="default.profraw" ./bin/c_practice
/opt/homebrew/opt/llvm/bin/llvm-profdata merge -sparse default.profraw -o default.profdata
/opt/homebrew/opt/llvm/bin/llvm-cov show ./bin/c_practice    -instr-profile=default.profdata -format=html -output-dir=cov
open cov/index.html
