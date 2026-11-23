# My Preprocessor

## Project Description
My Preprocessor is a custom C preprocessor tool developed using the C programming language.

It processes a C source file and generates an extended source file with a `.i` extension.

## Features
1. Comment Removal – Removes single-line and multi-line comments.
2. File Inclusion – Processes header file inclusion.
3. Macro Substitution – Replaces defined macros with their corresponding values.
4. Output Generation – Generates the preprocessed `.i` file.

## Technologies Used
- Programming Language: C
- Operating System: Linux (Ubuntu)
- Compiler: GCC
- Build Tool: Makefile

## Project Files
- `preprocessor.c` – Preprocessor implementation.
- `preprocessor.h` – Header file.
- `inputfile.c` – Input C source file.
- `inputfile.i` – Generated preprocessed output.
- `Makefile` – Compilation instructions.

## Compilation and Execution

```bash
make
./preprocessor
```

## Expected Output
The preprocessor processes the input C file and generates an extended `.i` file.

## Author
Varun Kumar S
