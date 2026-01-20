
# 🚀 GPU-Accelerated Raytracer

### THU Software Project · Team JK FlipFlop

Welcome to the official repository for **JK FlipFlop's Raytracer**, a high-performance rendering engine developed as part of the "Softwareprojekt" module at **Ulm University of Applied Sciences (THU)**.

This project tackles the mathematical and technical challenge of simulating physical light transport. It implements a hybrid rendering pipeline using **OpenGL Compute Shaders** to achieve realistic lighting effects like shadows, reflections, and refractions.

---

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

## 🎮 Controls

Once the application is running, you can navigate the scene using the following controls:

| Key | Action |
| --- | --- |
| **W, A, S, D** | Move Camera (Forward, Left, Backward, Right) |
| **Space** | Move Up |
| **Shift** | Move Down |
| **Mouse Drag** | Rotate View (Yaw/Pitch) |
| **UI Panel** | Toggle "Raytrace" button to start rendering 

 |

---

## 👥 The Team: JK FlipFlop

This project was realized by a team of 8 students:

* **Niklas Kümmel** (Product Owner, Backend)
* **Serhat Gürel** (Scrum Master, UI)
* **Jeremy Diem** (UI, Concepts)
* **Gabriel Penkert** (UI, Logging, Documentation)
* **Lovro Lupis** (Architecture, Engine Core)
* **Alton Bekolli** (Shader, Import/Export)
* **Felix Kussmann** (Camera System)
* **Valentin Talmon-l'Armée** (JSON Parser, Data Logic)

---

*Developed at Ulm University of Applied Sciences (THU), 2024.*
