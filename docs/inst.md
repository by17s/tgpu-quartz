# Instruction Set Architecture (ISA)

## General Syntax

```text
inst rd, r1, r2, r3, r4
```

---

## Scalar Arithmetic Instructions

| Instruction | Operation           | Description                  | Syntax                     |
| ----------- | ------------------- | ---------------------------- | -------------------------- |
| `fma`       | `rd = r1 * r2 + r3` | Fused multiply-add operation | `fma rd, r1, r2, r3`      |
| `add`       | `rd = r1 + r2`      | Adds two registers           | `add rd, r1, r2`          |
| `sub`       | `rd = r1 - r2`      | Subtracts `r2` from `r1`     | `sub rd, r1, r2`          |
| `mul`       | `rd = r1 * r2`      | Multiplies two registers     | `mul rd, r1, r2`          |
| `div`       | `rd = r1 / r2`      | Divides `r1` by `r2`         | `div rd, r1, r2`          |
| `sqrt`      | `rd = sqrt(r1)`     | Square root                  | `sqrt rd, r1`             |
| `min`       | `rd = min(r1, r2)`  | Minimum value                | `min rd, r1, r2`          |
| `max`       | `rd = max(r1, r2)`  | Maximum value                | `max rd, r1, r2`          |
---

## Bitwise & Shift Instructions

| Instruction | Operation                 | Description         | Syntax                    |
| ----------- | ------------------------- | ------------------- | -------------------------- |
| `and`       | `rd = r1 & r2`            | Bitwise AND         | `and rd, r1, r2`          |
| `or`        | `rd = r1 \| r2`           | Bitwise OR          | `or rd, r1, r2`           |
| `xor`       | `rd = r1 ^ r2`            | Bitwise XOR         | `xor rd, r1, r2`          |
| `not`       | `rd = ~r1`                | Bitwise NOT         | `not rd, r1`              |
| `shl`       | `rd = r1 << r2`           | Logical left shift  | `shl rd, r1, r2`          |
| `shr`       | `rd = r1 >> r2`           | Logical right shift | `shr rd, r1, r2`          |
| `mov`       | `rd = r1`                 | Register move       | `mov rd, r1`              |
| `xchg`      | `temp=r1; r1=r2; r2=temp` | Exchange registers  | `xchg rd, r1, r2`         |
---

## Control Flow Instructions

| Instruction | Operation               | Description            | Syntax                    |
| ----------- | ----------------------- | ---------------------- | ------------------------- |
| `bra`       | `PC = PC + offset`      | Unconditional branch   | `bra offset`              |
| `beq`       | `if (r1 == r2)`         | Branch if equal        | `beq r1, r2, offset`      |
| `bne`       | `if (r1 != r2)`         | Branch if not equal    | `bne r1, r2, offset`      |
| `blt`       | `if (r1 < r2)`          | Branch if less         | `blt r1, r2, offset`      |
| `bgt`       | `if (r1 > r2)`          | Branch if greater      | `bgt r1, r2, offset`      |
| `call`      | `CALL function_address` | Function call          | `call function_address`   |
| `ret`       | `RETURN`                | Function return        | `ret`                     |
| `sync`      | —                       | Thread synchronization | `sync`                    |

---

## Memory Instructions

### Global Memory

| Instruction | Operation                  | Syntax                   |
| ----------- | -------------------------- | -------------------------- |
| `ld_global` | `rd = GLOBAL_MEM[r1 + r2]` | `ld_global rd, r1, r2`    |
| `st_global` | `GLOBAL_MEM[r1 + r2] = r3` | `st_global rd, r1, r2`    |

### Local Memory

| Instruction | Operation                 | Syntax                   |
| ----------- | ------------------------- | -------------------------- |
| `ld_local`  | `rd = LOCAL_MEM[r1 + r2]` | `ld_local rd, r1, r2`     |
| `st_local`  | `LOCAL_MEM[r1 + r2] = r3` | `st_local rd, r1, r2`     |

---

## Bit Manipulation / Packing

| Instruction | Operation     | Syntax                   |
| ----------- | ------------- | ------------------------ |
| `mv8.l16`   | lower 8 bits  | `mv8.l16 rd, r1`         |
| `mv8.h16`   | upper 8 bits  | `mv8.h16 rd, r1`         |
| `mv16.l32`  | lower 16 bits | `mv16.l32 rd, r1`        |
| `mv16.h32`  | upper 16 bits | `mv16.h32 rd, r1`        |
| `mv32.l64`  | lower 32 bits | `mv32.l64 rd, r1`        |
| `mv32.h64`  | upper 32 bits | `mv32.h64 rd, r1`        |

---

## Floating-Point Conversions

| Instruction   | Conversion  |
| ------------- | ----------- |
| `mv32to16.fp` | FP32 → FP16 |
| `mv16to32.fp` | FP16 → FP32 |
| `mv32to16.bf` | FP32 → BF16 |
| `mv16to32.bf` | BF16 → BF32 |

---

## Immediate Load Instructions

| Instruction | Width  | Syntax                  |
| ----------- | ------ | ------------------------ |
| `lconst.8`  | 8-bit  | `lconst.8 rd, imm8`      |
| `lconst.16` | 16-bit | `lconst.16 rd, imm16`    |
| `lconst.32` | 32-bit | `lconst.32 rd, imm32`    |
| `lconst.64` | 64-bit | `lconst.64 rd, imm64`    |

---

## Atomic Instructions

| Instruction | Operation                      | Description                |
| ----------- | ------------------------------ | -------------------------- |
| `atom.add`  | atomic add (returns old value) | Performs an atomic addition of the value in register r3 to the value at the global memory address calculated by adding the values in registers r1 and r2. |
| `atom.sub`  | atomic subtract                | Performs an atomic subtraction of the value in register r3 from the value at the global memory address calculated by adding the values in registers r1 and r2. |
| `atom.mul`  | atomic multiply                | Performs an atomic multiplication of the value in register r3 with the value at the global memory address calculated by adding the values in registers r1 and r2. |
| `atom.div`  | atomic divide                  | Performs an atomic division of the value at the global memory address calculated by adding the values in registers r1 and r2 by the value in register r3. |
| `atom.st`   | atomic store                   | Performs an atomic store of the value in register r3 to the global memory address calculated by adding the values in registers r1 and r2. |
| `atom.cas`  | compare-and-swap               | Performs an atomic compare-and-swap operation: if the value at the global memory address calculated by adding the values in registers r1 and r2 equals the value in register r3, it is replaced with the value in register r4. |
| `atom.m.add` | `rdm = GLOBAL_MEM[r1+r2]; GLOBAL_MEM[r1+r2] += rdm` | Performs an atomic addition of the matrix in matrix register `rdm` to the matrix at the global memory address calculated by adding the values in registers `r1` and `r2`. |
| `atom.m.sub` | `rdm = GLOBAL_MEM[r1+r2]; GLOBAL_MEM[r1+r2] -= rdm` | Performs an atomic subtraction of the matrix in matrix register `rdm` from the matrix at the global memory address calculated by adding the values in registers `r1` and `r2`. |
| `atom.m.mul` | `rdm = GLOBAL_MEM[r1+r2]; GLOBAL_MEM[r1+r2] *= rdm` | Performs an atomic multiplication of the matrix in matrix register `rdm` with the matrix at the global memory address calculated by adding the values in registers `r1` and `r2`. |
| `atom.m.ld`  | `rdm = GLOBAL_MEM[r1+r2]` | Performs an atomic load of a matrix from the global memory address calculated by adding the values in registers `r1` and `r2` into matrix register `rdm`. |
| `atom.m.st`  | `GLOBAL_MEM[r1+r2] = rdm` | Performs an atomic store of a matrix from matrix register `rdm` to the global memory address calculated by adding the values in registers `r1` and `r2`. |
---

## Vector Instructions

### Integer Vectors (`i32`)

| Instruction      | Operation        |
| ---------------- | ---------------- |
| `vadd.i32`       | element-wise add |
| `vsub.i32`       | element-wise sub |
| `vmul.i32`       | element-wise mul |
| `vdiv.i32`       | element-wise div |
| `vmov.i32`       | move             |
| `vld_local.i32`  | load local       |
| `vst_local.i32`  | store local      |
| `vld_global.i32` | load global      |
| `vst_global.i32` | store global     |

---

### FP16 / FP32 / BF16 / BF32 Vectors

* `vadd.*`
* `vsub.*`
* `vmul.*`
* `vdiv.*`
* `vmov.*`
* `vld_local.*`
* `vst_local.*`
* `vld_global.*`
* `vst_global.*`

| Instruction      | Operation        |
| ---------------- | ---------------- |
| `vadd.*`       | element-wise add |
| `vsub.*`       | element-wise sub |
| `vmul.*`       | element-wise mul |
| `vdiv.*`       | element-wise div |
| `vmov.*`       | move             |
| `vld_local.*`  | load local       |
| `vst_local.*`  | store local      |
| `vld_global.*` | load global      |
| `vst_global.*` | store global     |

---

# Matrix Instructions

> [!IMPORTANT] 
> Matrix registers have no fixed type. The type of the matrix depends on the instruction used.  
> Syntax: `wmma.T rdm, rm1, rm2, rm3` where `T` can be: `i32`, `fp32`, `fp16`, `bf32`, `bf16`.

| Instruction | Syntax | Description |
|-------------|--------|-------------|
| `mldv.*`   | `rd[*] = r1` | Load vector from `r1` to `rd.*`. Where `*` is the vector element index (`x|y|z|w`). |
| `mstv.*`   | `rd = r1[*]` | Store vector from `r1.*` to `rd`. Where `*` is the vector element index (`x|y|z|w`). |
| `mvz.*`    | `rd[*] = 0` | Set vector element(s) to zero. |
| `maz.*`    | `rd = 0` | Set all vector elements to zero. |
| `madd.T`   | `rd = r1 + r2` | Performs element-wise addition of matrices stored in registers `r1` and `r2`, storing the result in `rd`. |
| `msub.T`   | `rd = r1 - r2` | Performs element-wise subtraction of matrices stored in registers `r1` and `r2`, storing the result in `rd`. |
| `mmul.T`   | `rd = r1 * r2` | Performs matrix multiplication of matrices stored in registers `r1` and `r2`, storing the result in `rd`. |
| `mxchg`  | `temp = r1; r1 = r2; r2 = temp` | Exchanges the contents of matrix registers `r1` and `r2`. |
| `mmov`  | `rd = r1` | Copies the contents of matrix register `r1` into matrix register `rd`. |
| `wmma.T`   | `rd = r1 * r2 + r3` | Performs a fused multiply-add operation on matrices stored in registers `r1` and `r2` with accumulation in `r3`, storing the result in `rd`. |


