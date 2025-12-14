## Overview
This document provides a complete list of registers for the TGPU-quartz GPU architecture, designed to support a wide range of tasks: high-performance computing (HPC), artificial intelligence (AI/ML), and 3D ray-traced graphics (RT).

The architecture includes **scalar, vector,** and **specialized control** registers.

| Category | Count | Examples | Purpose |
| --- | --- | --- | --- |
| **Scalar** | 30 | `ri32`, `rfp32`, `rbf16` | Basic computation and storage of single values. |
| **Vector** | 12 | `rv4fp32`, `rv4i32` | Accelerating parallel SIMD/SIMT operations. |
| **Control** | 13 | `rcpr`, `rrt_bvh_base` | Managing flow control, memory addressing, and RT traversal. |

---

## Registers (Data Registers, excluding RT)
Data registers are used to store operands and results of arithmetic and logical operations.

### Scalar Integer Registers
| Name | Size (bits) | Description |
| --- | --- | --- |
| `riz` | 64 | Zero register. |
| `ri8a-ri8h` | 8 | A register for storing 8-bit integer data. |
| `ri16a-ri16h` | 16 | A register for storing 16-bit integer data. |
| `ri32a-ri32h` | 32 | A register for storing 32-bit integer data. |
| `ri64a-ri64d` | 64 | A register for storing 64-bit integer data. |

### Scalar Floating-Point Registers
| Name | Size (bits) | Description |
| --- | --- | --- |
| `rfp16a-rfp16h` | 16 | A register for storing 16-bit half-precision floating-point data (FP16). |
| `rfp32a-rfp32h` | 32 | A register for storing 32-bit single-precision floating-point data (FP32). |
| `rbf16a-rbf16h` | 16 | A register for storing 16-bit BFloat data (BF16). |
| `rbf32a-rbf32h` | 32 | A register for storing 32-bit BFloat data (BF32). |

### Vector RegistersThese registers are used for the simultaneous processing of multiple scalar values (SIMD/SIMT).

| Name | Size (bits) | Elements | Description |
| --- | --- | --- | --- |
| `rv4fp16a-rv4fp16d` | 64 | 4x FP16 | A vector register storing 4 elements of 16-bit FP data. |
| `rv4fp32a-rv4fp32d` | 128 | 4x FP32 | A vector register storing 4 elements of 32-bit FP data. |
| `rv4bf32a-rv4bf32d` | 128 | 4x BF32 | A vector register storing 4 elements of 32-bit BF data. |
| `rv4i32a-rv4i32d` | 128 | 4x INT32 | A vector register storing 4 elements of 32-bit integer data. |

---

## Ray Tracing (RT Registers)
Specialized registers dedicated to accelerating the traversal and intersection of Bounding Volume Hierarchy (BVH) structures for Ray Tracing.

| Name | Size (bits) | Type | Purpose |
| --- | --- | --- | --- |
| `rrt_bvh_base` | 64 | Address | **BVH Base Address.** Stores a pointer to the root node of the entire BVH hierarchy in global memory. |
| `rrt_stack_ptr` | 64 | Address | **BVH Traversal Stack Pointer.** Stores the address of the current BVH node for navigation. |
| `rrt_hit_t` | 32 | FP32 | **Intersection Distance t.** Stores the distance from the ray origin to the hit point. |
| `rrt_t_min` | 32 | FP32 | Minimum ray distance for intersection search. |
| `rrt_t_max` | 32 | FP32 | Maximum ray distance for intersection search. |
| `rrt_prim_id` | 32 | ID | **Primitive ID.** Unique identifier of the triangle/geometry that was hit. |
| `rvrt_uv` | 64 | 2xFP32 | **Barycentric Coordinates (U, V).** Coordinates inside a primitive for texture or normal interpolation. |

---

## Control (General Control Registers)
These registers are used for managing instruction execution flow at the subgroup (warp/subgroup) or Execution Unit (EU) level.

| Name | Size (bits) | Description |
| --- | --- | --- |
| `rcpr` | 16 | **Predicate Register.** A bitmask of thread activity within a subgroup, used for branch divergence (`if/else`). |
| `rcsr` | 64 | **Status Register.** Stores arithmetic flags (overflow, zero, sign) for logical transitions. |
| `rclr` | 64 | **Loop Counter Register.** Used for efficient iteration counting in loops. |
| `rcar` | 64 | **Instruction Address Register.** Stores the current instruction address being executed by the thread group. |

---

## Control (Per Thread/Block Context Registers)
These registers store the context necessary for each thread and block to manage memory access and identification within the highly parallel architecture.

| Name | Size (bits) | Type | Description |
| --- | --- | --- | --- |
| `rtid` | 32 | ID | **Thread ID Register.** Stores the unique index of the thread within its workgroup. |
| `rbid` | 32 | ID | **Block ID Register.** Stores the coordinates (ID) of the workgroup in the grid. |
| `rtbase` | 64 | Address | **Local Memory Base Address.** The starting address of the memory area allocated for the current workgroup (Shared Memory). |
| `rtoff1` | 64 | Address | **Offset Register 1.** Used for calculating effective memory addresses during load/store operations. |
| `rtoff2` | 64 | Address | **Offset Register 2.** Used for calculating effective memory addresses during load/store operations. |
