# Architecture Overview

The TGPU-quartz architecture is designed for high-performance GPU computing with a focus on efficiency and scalability. It features a modular design that allows for flexible configuration of execution units, memory systems, and interconnects.

## Core Components

- **Execution Units (EUs)**: These are the primary computational units responsible for executing instructions.
- **Ray Tracing Units (RTUs) (in dev)**: Specialized units for accelerating ray tracing operations.
- **Memory System**: Includes both local and global memory hierarchies to support efficient data access patterns.
- **Interconnects**: High-speed communication pathways between EUs and memory systems.

## Execution Unit Details

Each EU contains:
- A set of registers for temporary storage
- ALU (Arithmetic Logic Unit) for computations
- Control unit to manage instruction execution
- Local memory space for fast access to frequently used data

## Ray Tracing Unit (RTU) Details (in dev)
The RTU is designed to handle ray tracing workloads efficiently. It includes:
- BVH Traversal Engine: Accelerates the traversal of Bounding Volume Hierarchies.
- Intersection Engine: Computes ray-object intersections.
- Specialized Registers: Dedicated registers for managing ray tracing state and data.
