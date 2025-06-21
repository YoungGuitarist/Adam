# 🧬 Adam

**Adam** — is an experimental C++ project implementing learning based on **genetic algorithms**. The system uses **random mutations** and **natural selection** to evolve a "population" of solutions, selecting the best units based on their `fitness`-status.

---

## 📌 About project

Adam - models the evolution process: generates a set of solutions (units), randomly mutates them, evaluates them according to the fitness criterion, and then saves and develops the most successful ones.

This can be used as a basis for:
- optimization problems;
- unsupervised machine learning;
- artificial life simulations;
- experiments with evolutionary strategies.

---

## Loging
This project have loging function, that helping to check statisticks and genome of simulation

Example:
```txt
Current day: 0
Unit 0:
   Day of death: 0
   Current HP: -8
   Genome: 
      HP:42
      SPEED: 64
Unit 2:
   Day of death: 0
   Current HP: 0
   Genome: 
      HP:64
      SPEED: 42
------------------------------------------------------- 
```

---

## 🔧 Installation and build

> **Requirements:**  
> - CMake (version 3.10+)  
> - Compiler C++ (g++, clang, MSVC и др.)

1. Clone repo:
```bash
git clone https://github.com/YoungGuitarist/Adam.git
cd Adam
```

2. Make `build/` dir and go in it:
```bash
mkdir build/
cd build/
```

3. Initialize Cmake and build it:
```bash
cmake ..
make
```

4. Run project:
```bash
./Adam
```
