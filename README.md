# NJVM - Ninja Virtual Machine 🥷💻

This repository contains the implementation of the **Ninja Virtual Machine (NJVM)** in **C**, designed to execute **Ninja bytecode**.

## 🛠 Technologies & Concepts
- **C** – Low-level programming
- **Binary Processing** – Parsing and executing binary code
- **Memory Management** – Dynamic allocation & garbage collection
- **Stack-based VM** – Execution using an operand stack

## 🚀 Usage
### 🔨 Build NJVM
```sh
./mknjvm 
```
### ▶️ Run NJVM with a Ninja Binary
```bash
./njvm <binary.bin>
```
## 📂 Project Structure
```
NJVM-Ninja-Virtual-Machine
│── mknjvm          # Build script for NJVM
│── njvm            # Compiled executable (created by mknjvm)
│── src/            # Source code directory
│   ├── njvm.c      # Main NJVM implementation
│   ├── Instruction.h # Header file with instruction definitions
│   ├── bigint/     # BigInt dependency
│── .git/           # Git repository (not uploaded)
│── README.md       # This file
```