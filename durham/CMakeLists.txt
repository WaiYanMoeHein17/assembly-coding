cmake_minimum_required(VERSION 3.10)
project(durham)

set(CMAKE_CXX_STANDARD 20)

# Enable NASM
enable_language(ASM_NASM)
set(CMAKE_ASM_NASM_OBJECT_FORMAT win64)

# Add assembly library
# add_library(optimized_ops div3.asm)

# Add main executable and link with assembly
add_executable(durham 
    src/main.cpp
    src/tokenizer.cpp
    src/parser.cpp
    src/gen_asm.cpp
    )
    
# target_link_libraries(durham PRIVATE optimized_ops)