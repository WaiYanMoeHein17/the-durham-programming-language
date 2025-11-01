# Durham Language Support for VS Code

Voice-enabled extension for the Durham programming language with real-time text-to-speech feedback.

## Features

- **Syntax highlighting** for `.dur` files
- **IntelliSense autocomplete** with Durham keywords
- **Text-to-speech** voices words as you type them
- **Smart typo detection** using Levenshtein distance algorithm

## Installation

### From VSIX (Recommended)
1. Download `durham-language-0.0.1.vsix`
2. Open VS Code Extensions view (Ctrl+Shift+X)
3. Click "..." menu → "Install from VSIX..."
4. Select the downloaded `.vsix` file
5. Reload VS Code

### From Source
1. Open this folder in VS Code
2. Run `npm install` to install dependencies
3. Run `npm run compile` to build
4. Press F5 to launch Extension Development Host

## Usage

The extension provides real-time audio feedback as you type in `.dur` files:
- Type any Durham keyword and hear it spoken after 100ms pause
- Press space, period, or newline to hear the complete word immediately
- Get autocomplete suggestions with Tab or Enter

## Keywords Supported

### Control Flow
- `if`, `while`, `for`

### I/O
- `tlc` (print), `mcs` (return)

### Operators
- Logical: `and`, `or`, `not`
- Comparison: `is` (assignment/equals), `at` (array access)
- Arithmetic: `durham` (+), `newcastle` (-), `york` (*), `edinburgh` (/)

### Memory & Structure
- `new`, `college` (vector/array type)
- `begin`, `end` (parentheses)
- `front`, `back` (braces)

### Numbers (Base-17: 0-16)
- `butler` (0), `chads` (1), `marys` (2), `collingwood` (3), `johns` (4)
- `castle` (5), `cuths` (6), `trevs` (7), `aidans` (8), `snow` (9)
- `grey` (10), `stephenson` (11), `hatfield` (12), `hildbede` (13)
- `south` (14), `vanmildert` (15), `ustinov` (16)

## About Durham

Durham is a Turing-complete programming language that uses Durham University college names as its base-17 numeric system. The language compiles to x86-64 assembly (NASM) for Windows.
