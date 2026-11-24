# Course Scheduling - COUNT, ROOMS, ENROLLMENTS

**Team members:** Asaf Nachman, Yuval Mitrani, Eliran Keinan

C++ program that schedules one day of courses into classrooms and solves three
optimization problems:

| Part | Goal | Method |
|------|------|--------|
| `COUNT` | Max number of non-overlapping courses in one room | Greedy, earliest finish time |
| `ROOMS` | Schedule all courses using the minimum number of rooms | Greedy by start time, free room with earliest last finish |
| `ENROLLMENTS` | Max total enrollments of non-overlapping courses in one room | Dynamic programming (weighted interval scheduling) |

Both sort orders are produced by a hand-written recursive merge sort.

## Files

| File | Role |
|------|------|
| `Course.h` | The `Course` struct (include guard). |
| `Algorithms.h` | All required function signatures plus helpers. |
| `Algorithms.cpp` | Merge sort (two orders), greedy COUNT, room allocation, DP (scan and binary-search variants share one DP/reconstruction helper), the three exhaustive checkers. No `main`. |
| `main.cpp` | Main program: reads input from stdin, runs the algorithms, prints `COUNT`, `ROOMS`, `ENROLLMENTS`. |
| `tests.cpp` | doctest test suite (doctest supplies `main`). |
| `doctest.h` | doctest single-header library (MIT license, unmodified). |
| `testdata/` | Input/expected-output pairs: `sample`, `empty`, `ties` (from the assignment) and `predecessor` (exposes the p(j) bug). |

## Input format

```
n
<id> <start> <finish> <enrollments>   (n lines)
```
Times are minutes from the start of the day (09:00 = 540). Ids are English, no spaces.

## Build and run

Requires only a C++17 compiler (tested with **g++ 13.3.0**, Ubuntu 24.04).
External dependency: `doctest.h` **v2.5.0** (single header, MIT license, included
in the repository; downloaded from
`https://raw.githubusercontent.com/doctest/doctest/master/doctest/doctest.h`).

Tested result: `./tests` reports 20 test cases and 298216 assertions, all passed;
`sample`, `empty`, `ties` and `predecessor` outputs match the saved `.out` files.

### Main program
```bash
g++ -Wall -Wextra main.cpp Algorithms.cpp -o courses
./courses < testdata/sample.in
./courses < testdata/sample.in > my.out && diff my.out testdata/sample.out
```
Windows (Command Prompt):
```
g++ -Wall -Wextra main.cpp Algorithms.cpp -o courses.exe
courses.exe < testdata\sample.in
```

### Tests
```bash
g++ -Wall -Wextra tests.cpp Algorithms.cpp -o tests
./tests
```
Windows: `g++ -Wall -Wextra tests.cpp Algorithms.cpp -o tests.exe` then `.\tests.exe`.

Exit code 0 means all tests passed.

### Check all saved examples
```bash
for t in sample empty ties predecessor; do
  ./courses < testdata/$t.in | diff - testdata/$t.out && echo "$t OK"
done
```

## What the tests cover

* Merge sort: both orders, all tie-breaks, output is a permutation of the input, empty/single input.
* The three examples from the assignment, checked field by field.
* Edge cases: empty input, single course, touching endpoints, all overlapping,
  identical times with different ids, unsorted input (input not modified),
  count-vs-enrollments gap, DP tie rule, reusing a freed room, opening a new room.
* p(j) critique: a 3-course input where the "only check j-1" rule fails
  (correct p = {-1,-1,0}, buggy {-1,-1,-1}; optimum 15 vs 10).
* Randomized, reproducible (own LCG, fixed seeds):
  * seed `20261120`: 3000 inputs with 0-5 courses, times 0-20, enrollments 1-6,
    compared with all three exhaustive solutions plus validity checks.
  * seed `424242`: 300 inputs with 6-200 courses, times 0-1440, enrollments 1-1000,
    validity checks only; number of rooms compared with the maximum overlap depth;
    scan and binary-search DP compared (same p array and identical lists).
* Validity: no overlaps, no duplicate ids, records unchanged from input, every
  course exactly once in ROOMS, no empty room.

Passing tests on finite inputs does not prove correctness; the proofs are in the reports.
