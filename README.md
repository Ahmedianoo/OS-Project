# 🖥️ OS Scheduler with Buddy Memory Manager

This project implements an **Operating System Scheduler** in C with three classic scheduling algorithms, integrated with a **Buddy System memory allocator**.  
It simulates process creation, scheduling, context switching, and memory allocation using **inter-process communication** and a simulated clock.

---

## 🚀 Features

### 🧵 Scheduling Algorithms
1. **Highest Priority First (HPF)**  
   - Non-preemptive  
   - Uses a priority queue (lower number = higher priority)  
   - Once a process starts, it runs until completion  

2. **Shortest Remaining Time Next (SRTN)**  
   - Preemptive  
   - Uses remaining time as priority (shorter = higher priority)  
   - If two processes have the same remaining time, the *new* process is chosen  

3. **Round Robin (RR)**  
   - Preemptive  
   - Uses a circular queue  
   - Fixed quantum (time slice), head traverses through processes  
   - Ensures fair execution for all processes  

---

### 🧩 Memory Management: Buddy System
- Total memory size: **1024 bytes**  
- Minimum block size: **8 bytes**  
- Maximum block size per process: **≤ 256 bytes**  
- Allocation rounded to the nearest power of two  
- Efficient splitting and merging of memory blocks  
- Processes wait in a queue if memory is full until space is freed  

---

## 📊 System Specifications
- **Architecture:** Single uni-core CPU  
- **IPC:** Message queues & signals  
- **Language:** C  
- **Environment:** Linux  
- **Process requirement:** Memory size stays constant throughout execution  

---

## **IPC & Simulation**:
  - System V **Message Queues** for process generator → scheduler
  - **Signals** for coordination/cleanup
  - Shared simulated **clock** process (`clk.out`)
  - Logs for scheduler events, memory ops, and performance

---


---

## 📥 Input Format (`processes.txt`)

The file contains process definitions (first line is a header and is ignored):

```
#id  arrival runtime priority memory
1    1       6      5       200
2    2       2      1       100
3    4       4      4       120
4    10      3      5       150
5    12      5      3       170
```

- **id**: Logical process ID  
- **arrival**: Arrival time in system  
- **runtime**: Execution time required  
- **priority**: Used in HPF (lower = higher priority)  
- **memory**: Memory size requested (bytes)

> `process_generator.c` reads this file, initializes each PCB, and sends it to the scheduler at the correct clock tick via a message queue.

---

## ⚙️ How It Works (High Level)

### Process Generator (`process_generator.c`)
- Reads `processes.txt` → array of `PCB`s (sets `remainingTime`, flags, etc.).
- Forks the **scheduler** (`scheduler.out`) and **clock** (`clk.out`).
- Optionally sorts for SRTN (`qsort` with `comparePCBForSRTN`).
- Uses `initClk()` and sends each process to the scheduler when `arrivalTime <= getClk()` using a **message queue**.
- Handles cleanup via `SIGINT` (`clearResources`).

### Scheduler (`scheduler.c`)
- Receives `PCB`s via message queue.
- Chooses the next process according to the selected **algorithm**:
  - **HPF** – Non‑preemptive, priority queue.
  - **SRTN** – Preemptive, remaining time priority.
  - **RR** – Preemptive, circular queue with quantum.
- Tracks `startTime`, `finishTime`, `waitingTime`, **WTA**, etc.
- Allocates/frees memory for each process using the **Buddy** allocator.

### Memory Manager (`memory.c`)
- Total memory: **1024 bytes**; min block: **8 bytes**; max per process: **≤ 256 bytes**.
- Rounds requests to the nearest power of two.
- Splits/merges (buddy) blocks on allocation/free.
- Exposes debug printing to visualize the memory map.

---

## 🔑 Key Functions & Their Role (Buddy Allocator)

| Function               | Purpose                                                     |
|------------------------|-------------------------------------------------------------|
| `initializeMemory()`   | Initialize the root block (1024 bytes)                      |
| `nextPowerOfTwo(n)`    | Round request to nearest 2^k                                |
| `allocateBlock(...)`   | Allocate best-fit block; split as needed                    |
| `splitLeaf(...)`       | Split a block into two equal-sized buddies                  |
| `freeBlock(...)`       | Mark a block free; attempt to merge with its buddy          |
| `merge(...)`           | Merge two free buddy blocks into a larger free block        |
| `printMemoryBlocks()`  | Print current memory layout (debug/visualization)           |

---

## 🧩 Data Structures

### Process Control Block (PCB)
```c
typedef struct PCB {
    int processID;
    pid_t processPID, processPPID;
    bool isFirstRun, isStopped;
    int processPriority;
    int arrivalTime, runtime, remainingTime;
    int finished, forked;
    int pStart, startTime, finishTime, LastExecTime, last_scheduled_time;
    int waitingTime, turnAroundTime;
    float weightedTurnAroundTime;
    int memorysize, freed;
    MemoryBlock *memPtr;
} PCB;
```

### Algorithm Enum
```c
enum algorithms {
    HELPER,
    HPF,
    SRTN,
    RR
};
```

---

## 🛠️ Build & Run (via Makefile)

Use the provided **Makefile** targets:

```make
build:
\tgcc process_generator.c -o process_generator.out
\tgcc clk.c -o clk.out
\tgcc scheduler.c -o scheduler.out -lm
\tgcc process.c -o process.out
\tgcc test_generator.c -o test_generator.out

clean:
\trm -f *.out  processes.txt

all: clean build

run:
\t./process_generator.out

test:
\t./test_generator.out
```

### Typical workflow
```bash
make          # builds all .out files
make run      # runs the process generator (will prompt for algorithm)
# or
./process_generator.out
```

- When prompted, choose: `1` = **HPF**, `2` = **SRTN**, `3` = **RR**.  
- If **RR** is selected, enter the **quantum** (must be > 0).

> `process_generator.out` handles forking the scheduler and clock automatically, sends PCBs at their arrival times, and cleans up resources on exit.

---

## 📸 Screenshots

Upload your images (e.g., to `screenshots/`) and replace the paths below.

### 🔹 HPF Timing Diagram
<img src="https://github.com/Ahmedianoo/OS-Project/releases/download/v1.0.0/HPF.jpg" width="700" alt="HPF Timing Diagram"/>

### 🔹 SRTN Timing Diagram
<img src="https://github.com/Ahmedianoo/OS-Project/releases/download/v1.0.0/SRTN.jpg" width="700" alt="SRTN Timing Diagram"/>

### 🔹 RR Timing Diagram
<img src="https://github.com/Ahmedianoo/OS-Project/releases/download/v1.0.0/RR.jpg" width="700" alt="RR Timing Diagram"/>

### 🔹 Performance Output
<img src="https://github.com/Ahmedianoo/OS-Project/releases/download/v1.0.0/04.png" width="700" alt="Performance Table"/>

---

---

## 👤 Author

**Ahmed Mohamed**  
📧 [ahmed.mohamed04@hotmail.com](mailto:ahmed.mohamed04@hotmail.com)  
🔗 [LinkedIn Profile](https://www.linkedin.com/in/ahmed04/)

---

## 🏷️ Tags
`#OperatingSystems` `#Scheduler` `#HPF` `#SRTN` `#RoundRobin` `#BuddyAllocator` `#CProgramming` `#IPC` `#Linux`
