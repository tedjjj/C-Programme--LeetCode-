# Block-Based Social Network Friendship Ratings System (SFSD Project)

A low-level C implementation simulating a **Database Management System (DBMS)** disk storage engine. This project was developed as part of the **SFSD** (*Structure de Fichiers et Structure de Données*) curriculum. It implements custom disk-block storage architectures to manage a social network friendship rating database efficiently while minimizing disk I/O operations.

## 📌 Project Overview
The program manages a social network database where students rate their friendship with other students (ratings range from `-1` to `+3`). Each evaluation is recorded with a Unix timestamp. Because relationships change over time, the system stores history and queries the **most recent rating** between any two students using timestamps.

---

## 🛠️ Storage Architecture & Design
Rather than loading the entire database into memory or rewriting the whole file during updates, this project implements a custom page-block storage engine using two binary files:

### 1. TOF (Trier Ordonné Fixe) — Primary File
* **Block Capacity:** 32 records (`BLOCK_CAPACITY = 32`).
* **Sorting:** Records are kept physically sorted by sender (`from`) and receiver (`to`) IDs.
* **Loading Factor:** Initially loaded at **75% capacity** (24 records per block) to reserve empty space for quick, in-place insertions.
* **Metadata:** Each block contains pointers to the head and tail of its overflow chain in the LNOF file.

### 2. LNOF (Liste Non Ordonnée Fichier) — Overflow File
* **Chained Overflow:** When a primary TOF block becomes completely full, new records belonging to that block are appended to an overflow chain in the LNOF file.
* **Block Capacity:** 5 records (`LNOF_BLOCK_CAPACITY = 5`).
* **Space Management:** Implements a **free list** (linked list of deleted or available blocks) to recycle storage space.

---

## 🔍 Problems Solved by this Project

* **Disk I/O Efficiency:** In real database systems, disk reads/writes are the main performance bottleneck. This program uses a simulated **Abstract Machine** (`TOF_readBlock` / `TOF_writeBlock`) to load and write fixed-size blocks (pages) only when necessary, minimizing expensive disk I/O.
* **The Block Overflow Problem (ISAM):** Demonstrates how to handle insertions when sorted file pages are full. Instead of shifting records across the entire database, the program links overflow chains (similar to ISAM / Indexed Sequential Access Method overflow pages).
* **Fragmentation & Maintenance (Compaction):** Over time, heavy insertions fill the overflow file, turning $O(\log N)$ binary searches into sequential scans. The project solves this by implementing an offline **Reorganization (Merge & Compact)** algorithm that consolidates TOF and LNOF back into a single sorted, overflow-free TOF file.
* **Version Control / Temporal Records:** Solves the challenge of keeping historical relationship logs while quickly resolving queries to retrieve the latest version of any record.

---

## 🚀 Key Features & Algorithms

1. **Initial Bulk Loading (Task 1):** Parses a raw text dataset (`Data.txt`) and populates the primary TOF binary file, applying the 75% loading factor.
2. **Binary Search Engine:**
   * **Internal Block Search:** Binary search (`recherche_block`) inside a loaded block.
   * **Global Block Search:** Binary search (`recherche_tof`) across the block indices of the TOF file.
3. **Record Insertion (Task 2):**
   * Locates the correct block via binary search.
   * If the block has space, it performs an in-place shift (`internal_shift`) to insert the record while keeping it sorted.
   * If the block is full, it dynamically requests/creates an overflow block in the LNOF file and appends the record.
4. **Latest Rating Query:**
   * Searches the primary block and its overflow chains, resolving and returning the record with the highest timestamp.
5. **Rating Update:**
   * Appends an updated evaluation with a new, higher timestamp into the block's overflow chain.
6. **Offline Reorganization (Task 3):**
   * Merges `Task1.txt` (TOF) and `overflow.txt` (LNOF) into a single array.
   * Sorts the records using Insertion Sort (primary sort: `to` ascending, secondary sort: `from` descending).
   * Writes the consolidated data into a clean, new TOF file (`merged.txt`) with all overflow chains removed and loading factors reset.

---

## 💻 Menu Options
The interactive console provides the following administrative commands:
1. **Initialize TOF File:** Load 50% of the dataset into `Task1.txt`.
2. **Insert N Records:** Simulate new user evaluations.
3. **Search for Rating:** Query the latest rating between two student IDs.
4. **Update a Rating:** Log a new friendship rating.
5. **Display Friends:** List all friends of a given student along with their latest ratings.
6. **Reorganize Files:** Compact files by merging TOF and LNOF.
7. **Display New TOF:** Inspect the reorganized `merged.txt`.
8. **Display Block Address:** Output a specific block's physical metadata and records.
9. **Display Entire TOF:** Print all blocks in the primary file.
10. **Display Entire LNOF:** Inspect all linked overflow pages.

---

## 📂 File Structure
* `structers.h` — Data types and low-level block I/O operations (Abstract Machine).
* `Task1.c` — Bulk loader.
* `Task2.c` — Insertion, update, binary searching, and friendship extraction.
* `task3.c` — Merge and compaction algorithm.
* `help_functions.c` — Console print helpers and address calculations.
* `main.c` — Interactive terminal user interface.
