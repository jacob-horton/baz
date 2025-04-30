# Baz

Baz is a language that I am designing and implementing for my dissertation.

Main aims:
- Familiar + easy to use/learn
- Expressive type system (similar to Rust's)

I have decided to output to C++ so that I can focus on making a good language and type system without worrying about an interpreter or machine code generation. This should be possible to swap out later though


# Building

To build the Baz transpiler on a Unix like system:
```bash
mkdir ./build
cd ./build

cmake ..
make baz
chmod +x baz
```

# Running

To compile a specific file, use the following:
```bash
./baz <input_file>
```

The outputted C++ file can then be compiled with:
```bash
g++ output.cpp -o main
```

Then executed with:
```bash
./main
```
