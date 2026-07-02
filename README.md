# 🚗 Traffic-Aware Navigation Engine

A high-performance C++ route planning system that computes optimal routes over dynamic road networks with live traffic updates. The engine combines graph algorithms, epoch-based cache synchronization, and custom data structures to efficiently process shortest-path queries while adapting to changing traffic conditions.

---

## ✨ Features

- 🚦 Dynamic traffic updates through real-time edge weight modifications.
- 🗺️ Computes optimal routes across weighted road networks.
- ⚡ Epoch-based cache synchronization using precomputed shortest paths.
- 📊 Automatically switches between cached and dynamic routing depending on traffic changes.
- 🚶 Supports multiple routing profiles (Car and Walking).
- 📚 Modular object-oriented design for extensibility.

---

## ⚙️ Architecture

```
                Traffic Updates
                       │
                       ▼
                 Road Network Graph
                       │
        ┌──────────────┴──────────────┐
        │                             │
        ▼                             ▼
 Epoch Cache (Floyd-Warshall)    Dynamic Routing
                                 (Dijkstra)
        │                             │
        └──────────────┬──────────────┘
                       ▼
               Optimal Route Query
```

---

## 📁 Project Structure

```
.
├── include
│   ├── Graph.hpp
│   ├── RoutingEngine.hpp
│   └── SkipList.hpp
│
├── src
│   └── main.cpp
│
├── test
│   └── map_data.txt
│
└── README.md
```

---

## 🧠 Design Overview

### Dynamic Traffic Updates

Traffic events continuously modify road costs during execution.

```
TRAFFIC A B Timestamp Multiplier
```

Each update changes the effective edge weight used during routing.

---

### Epoch-Based Cache

Instead of running Dijkstra for every request:

- Floyd–Warshall periodically computes all-pairs shortest paths.
- The resulting matrix is cached.
- If no traffic changes have occurred since the previous synchronization, queries are answered directly from the cache.
- Otherwise, the engine falls back to dynamic shortest-path computation.

This significantly reduces redundant computations for stable traffic conditions.

---

### Skip List

The project includes a custom Skip List implementation for efficient ordered data management and logarithmic-time operations.

---

## 🛠 Technologies

- C++
- Object-Oriented Programming
- Graph Data Structures
- Skip Lists
- Dijkstra's Algorithm
- Floyd–Warshall Algorithm
- Kosaraju's Algorithm

---

## 🚀 Building

```bash
g++ src/main.cpp -std=c++17 -O2 -o navigation
```

Run:

```bash
./navigation
```

---

## 📌 Example Workflow

```
Initialize Graph
        │
        ▼
Load Roads
        │
        ▼
Receive Traffic Updates
        │
        ▼
Synchronize Cache
        │
        ▼
Route Query
        │
        ├── Cache Valid
        │       │
        │       ▼
        │  Return Cached Distance
        │
        └── Cache Stale
                │
                ▼
         Run Dynamic Routing
```

---

## 📈 Highlights

- Real-time traffic-aware route optimization
- Epoch-based shortest-path cache
- Dynamic graph updates
- Profile-based routing
- Modular OOP architecture
- Efficient graph processing

---

