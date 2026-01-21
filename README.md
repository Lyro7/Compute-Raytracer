<p align="center">
<img width="250" height="250" alt="drtonipeperoni_Computer_circuit_board_wired_abstract_tech_software_label_JK_62c55c69-cdb9-4d4e-b8ac-37f78764dcc0" src="https://github.com/user-attachments/assets/d14d7a4f-f9aa-440b-b07d-8facb976ba34" />
</p>

# 🚀 GPU-Accelerated Raytracer

### THU Software Project · Team JK FlipFlop

Welcome to the official repository for **JK FlipFlop's Raytracer**, a high-performance rendering engine developed as part of the "Softwareprojekt" module at **Ulm University of Applied Sciences (THU)**.

This project tackles the mathematical and technical challenge of simulating physical light transport. It implements a hybrid rendering pipeline using **OpenGL Compute Shaders** to achieve realistic lighting effects like shadows, reflections, and refractions.

---
<img width="2143" height="1368" alt="image" src="https://github.com/user-attachments/assets/48ef3764-e137-4fa3-8c95-6f1a1a8cc54a" />

## 🎯 Project Overview

The goal of this project was to design and implement a powerful, GPU-based raytracer in C++. Unlike traditional CPU-based renderers, our engine leverages the parallel processing power of the GPU via Compute Shaders to render complex 3D scenes efficiently.

**Key Capabilities:**

* **Hybrid Rendering:** Switch seamlessly between a real-time Rasterization Preview and the high-fidelity Raytracing mode.

* **Physical Light Simulation:** Calculates interactions of light with virtual objects, including shadows and reflections.

* **Scene Management:** Full support for importing `.OBJ` models with `.MTL` materials and saving/loading scenes via JSON/ZIP.



---

## 📸 Features & UI

The application features a modern, intuitive **Graphical User Interface (GUI)** built with **ImGui**, inspired by professional IDEs.

### 🛠️ Interactive Scene Editing

* **Live Manipulation:** Modify object positions, rotations, and scaling in real-time.

* **Lighting Control:** Dynamically adjust light intensity, color, and position using visual placeholders.

* **Camera System:** Free-roaming camera with adjustable Field of View (FOV) and WASD navigation.



### 📂 Asset Pipeline

* **Smart Import:** Drag-and-drop or import `.OBJ` files; the system automatically triangulates geometry.

* **Export Tools:** Save your scene configuration as a JSON file or export the rendered result as high-res PNG/JPG images.

> [!TIP]
> **🚀 Performance Hint:**
> For smooth interaction while moving objects or lights, keep the resolution **low** (e.g., Preview Mode).
> Only switch to a **High Resolution** preset right before clicking the **Raytrace** button for your final image!

---

## 🏗️ Architecture & Tech Stack

This project is built on a robust C++ architecture designed for high-performance computing.


### System Flow

1. **Scene Loading:** The `SceneLoader` parses JSON/ZIP files and loads geometry via `TinyObjLoader`.

2. **Data Transfer:** Geometry and materials are packed into **SSBOs** (Shader Storage Buffer Objects) and uploaded to the GPU.

3. **Compute Shader:** The core raytracing logic (Möller-Trumbore intersection, shading) runs entirely on the GPU.

4. **Output:** The result is written to a texture and displayed in the viewport.


---

## 📦 Installation & Build

The project uses **CMake** for cross-platform build configuration.

**Prerequisites:**

* C++ Compiler (supporting C++17)
* OpenGL 4.3+ compatible GPU
* CMake

**Build Steps:**

```bash
# 1. Clone the repository
git clone https://github.com/YourUsername/Raytracer-JKFlipFlop.git
cd Raytracer-JKFlipFlop

# 2. Create build directory
mkdir build
cd build

# 3. Configure and Build
cmake ..
make

```

---

## 👥 The Team: JK FlipFlop

This project was realized by a team of 8 students:

* **Niklas Kümmel** (Product Owner, Backend)
* **Serhat Gürel** (Scrum Master, UI)
* **Jeremy Diem** (UI, File Explorer, Concepts)
* **Gabriel Penkert** (UI, Logging, Documentation)
* **Lovro Lupis** (Architecture, Engine Core)
* **Alton Bekolli** (Shader, JASON Parser, Import/Export)
* **Felix Kussmann** (Camera System)
* **Valentin Talmon-l'Armée** (JSON Parser, Data Logic)

---

*Developed at Ulm University of Applied Sciences (THU), 2025/2026.*
