# Content Moderation System - Max Heap Priority Queue

## Project Overview

Implementation of a **priority queue-based content moderation system** using a max-heap data structure. Designed to handle billions of posts per day, prioritizing high-risk content for human review while efficiently managing storage capacity.

**Problem**: Store posts with highest risk levels and process the riskiest ones first.

---

## Quick Start

### Installation (Ubuntu/Debian)
```bash
# Install SDL2 dependencies
sudo apt-get update
sudo apt-get install libsdl2-dev libsdl2-ttf-dev fonts-dejavu
```

### Build & Run
```bash
# Build everything
make

# Run interactive GUI (recommended for demo)
make run-gui

# Run performance tests
make run
```

---

## Interactive GUI Demo (`heap_gui`)

### Features
- **Text input**: 140-character posts (Twitter-style)
- **Automatic risk assessment**: Bot analyzes content in real-time
- **Visual queue**: Color-coded posts by risk level
  - 🟢 Green (0-40%): Low risk
  - 🟡 Yellow (40-70%): Medium risk
  - 🔴 Red (70-99%): High risk
  - ⛔ Deleted (100%): Automatically removed
- **Review button**: Process highest-risk post
- **Live statistics**: Posted, reviewed, deleted, evicted counts

### Demo Examples

**Low Risk (Green)**
```
Hello everyone! Having a great day.
```

**Medium Risk (Yellow)**
```
This is so STUPID! Why would anyone do this?
I hate waiting in line!
```

**High Risk (Red)**
```
I HATE this so much! This is complete shit!
KILL the competition with our BOMB deals!
```

**Automatic Deletion (100%)**
```
FUCK THIS SHIT! I HATE EVERYTHING! KILL KILL KILL!!!
```
→ Instantly deleted, never enters queue

### Controls
- **Type and press Enter** or click "POST"
- **Click "Review"** to process highest-risk post
- **Click "Clear"** to empty queue

---

## Performance Tests (`test_heap`)

Runs 7 comprehensive tests:

1. **Insertion with 100K posts** - Measures O(log n) insertion time
2. **Complexity growth** - Demonstrates logarithmic scaling
3. **Eviction performance** - Tests minimum post removal
4. **Memory analysis** - ~200 bytes per post
5. **Stress test** - 10 seconds of production traffic simulation
6. **Heap integrity** - Verifies max/min properties maintained

```bash
make run
```

**Expected Results**:
- Insertion: ~3 microseconds
- Scales to 1M+ posts efficiently
- Memory: ~200 bytes per post

---

## Testing

```bash
# Run all tests (functional + performance)
make run-test-suite

# Individual test suites
make run-test-heap      # Functional correctness
make run-test-perf      # Performance benchmarks

# Memory leak check
make valgrind-tests
```

All tests should pass: `100%: Checks: 10, Failures: 0`

---

## Key Features

✅ **O(log n) insertion** - Fast at scale  
✅ **O(1) max access** - Instant highest-risk retrieval  
✅ **Automatic deletion** - 100% risk posts never stored  
✅ **Smart eviction** - Removes lowest-risk when full  
✅ **Memory efficient** - Handles 1M+ posts  

---

## Algorithm Complexity

| Operation | Time | Space |
|-----------|------|-------|
| Insert | O(log K) | O(1) |
| Peek Max | O(1) | O(1) |
| Pop Max | O(log K) | O(1) |
| Evict Min | O(K) | O(1) |

*K = distinct risk levels (typically ~8,500 for 100K posts)*

---

## Demonstration Flow

1. **Start GUI**: `make run-gui`
2. **Post low-risk content** → See green color, enters queue
3. **Post high-risk content** → See red color, jumps to top
4. **Post extreme content** → Watch automatic deletion (100%)
5. **Click Review** → Processes highest-risk first
6. **Fill queue** → Show automatic low-risk eviction
7. **Show tests**: `make run` → Proves scalability to 1M posts

---

## Troubleshooting

**Font error**: Install DejaVu fonts
```bash
sudo apt-get install fonts-dejavu
```

---

## Project Structure

```
├── include/max_heap.h    # Heap interface
├── src/max_heap.c        # Implementation  
├── main.c                # Performance tests
├── main_sdl.c            # Interactive GUI
├── tests/                # Unit tests
└── Makefile              # Build system
```
