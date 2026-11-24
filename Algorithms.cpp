#include "Algorithms.h"

#include <cstddef>

namespace {

// ----------------------------- comparisons ---------------------------------

// Order for COUNT and ENROLLMENTS: finish, then start, then id.
bool lessByFinish(const Course& a, const Course& b) {
    if (a.finish != b.finish) return a.finish < b.finish;
    if (a.start != b.start) return a.start < b.start;
    return a.id < b.id;
}

// Order for ROOMS: start, then finish, then id.
bool lessByStart(const Course& a, const Course& b) {
    if (a.start != b.start) return a.start < b.start;
    if (a.finish != b.finish) return a.finish < b.finish;
    return a.id < b.id;
}

using Less = bool (*)(const Course&, const Course&);

// ----------------------------- merge sort ----------------------------------

// Merges the sorted ranges [lo, mid) and [mid, hi) of v using buffer tmp.
void mergeRanges(std::vector<Course>& v, std::vector<Course>& tmp,
                 std::size_t lo, std::size_t mid, std::size_t hi, Less less) {
    std::size_t i = lo, j = mid, k = lo;
    while (i < mid && j < hi) {
        // Take from the right half only if strictly smaller: keeps stability.
        if (less(v[j], v[i])) tmp[k++] = v[j++];
        else                  tmp[k++] = v[i++];
    }
    while (i < mid) tmp[k++] = v[i++];
    while (j < hi)  tmp[k++] = v[j++];
    for (std::size_t t = lo; t < hi; ++t) v[t] = tmp[t];
}

// Recursively sorts v[lo, hi).
void mergeSortRange(std::vector<Course>& v, std::vector<Course>& tmp,
                    std::size_t lo, std::size_t hi, Less less) {
    if (hi - lo <= 1) return;               // base case: 0 or 1 element
    std::size_t mid = lo + (hi - lo) / 2;
    mergeSortRange(v, tmp, lo, mid, less);
    mergeSortRange(v, tmp, mid, hi, less);
    mergeRanges(v, tmp, lo, mid, hi, less);
}

void mergeSortWith(std::vector<Course>& courses, Less less) {
    std::vector<Course> tmp(courses.size());
    mergeSortRange(courses, tmp, 0, courses.size(), less);
}

// True if no two courses in the list overlap.
bool pairwiseCompatible(const std::vector<Course>& list) {
    for (std::size_t a = 0; a < list.size(); ++a)
        for (std::size_t b = a + 1; b < list.size(); ++b)
            if (coursesOverlap(list[a], list[b])) return false;
    return true;
}

// Recursive search over all set partitions of courses into rooms.
// Each course goes into an existing room or opens exactly one new room,
// so partitions that differ only by room numbering are generated once.
void partitionSearch(const std::vector<Course>& courses, std::size_t index,
                     std::vector<std::vector<Course>>& rooms,
                     std::size_t& best) {
    if (rooms.size() >= best) return;       // cannot improve
    if (index == courses.size()) {
        bool valid = true;
        for (const auto& room : rooms)
            if (!pairwiseCompatible(room)) { valid = false; break; }
        if (valid) best = rooms.size();
        return;
    }
    for (std::size_t r = 0; r < rooms.size(); ++r) {
        rooms[r].push_back(courses[index]);
        partitionSearch(courses, index + 1, rooms, best);
        rooms[r].pop_back();
    }
    rooms.push_back(std::vector<Course>{courses[index]});
    partitionSearch(courses, index + 1, rooms, best);
    rooms.pop_back();
}

} // namespace

// ============================ public helpers ===============================

bool coursesOverlap(const Course& a, const Course& b) {
    return a.start < b.finish && b.start < a.finish;
}

long long totalEnrollments(const std::vector<Course>& courses) {
    long long sum = 0;
    for (const auto& c : courses) sum += c.enrollments;
    return sum;
}

void mergeSort(std::vector<Course>& courses) {
    mergeSortWith(courses, lessByFinish);
}

void mergeSortByStart(std::vector<Course>& courses) {
    mergeSortWith(courses, lessByStart);
}

// ================================ COUNT ====================================

std::vector<Course> selectCourses(const std::vector<Course>& sortedCourses) {
    std::vector<Course> chosen;
    bool any = false;
    int lastFinish = 0;
    for (const auto& c : sortedCourses) {
        if (!any || c.start >= lastFinish) {   // compatible with last chosen
            chosen.push_back(c);
            lastFinish = c.finish;
            any = true;
        }
    }
    return chosen;
}

// ============================= ENROLLMENTS =================================

std::vector<int> computePredecessorsScan(
    const std::vector<Course>& sortedCourses) {
    const int n = static_cast<int>(sortedCourses.size());
    std::vector<int> p(n, -1);
    for (int j = 0; j < n; ++j) {
        // Finish times are non-decreasing, so the first compatible course
        // found when scanning backwards is the last compatible one.
        for (int i = j - 1; i >= 0; --i) {
            if (sortedCourses[i].finish <= sortedCourses[j].start) {
                p[j] = i;
                break;
            }
        }
    }
    return p;
}

std::vector<int> computePredecessorsBinary(
    const std::vector<Course>& sortedCourses) {
    const int n = static_cast<int>(sortedCourses.size());
    std::vector<int> p(n, -1);
    for (int j = 0; j < n; ++j) {
        // Find the last index i in [0, j-1] with finish <= start of j.
        // Invariant: every index < lo satisfies the condition,
        //            every index >= hi (within [0, j)) does not.
        int lo = 0, hi = j;
        while (lo < hi) {
            int mid = lo + (hi - lo) / 2;
            if (sortedCourses[mid].finish <= sortedCourses[j].start) lo = mid + 1;
            else hi = mid;
        }
        p[j] = lo - 1;                      // -1 if no compatible course
    }
    return p;
}

std::vector<Course> selectByPredecessors(
    const std::vector<Course>& sortedCourses, const std::vector<int>& p) {
    const int n = static_cast<int>(sortedCourses.size());
    // M[k] = best total enrollments using only the first k courses (0..k-1).
    std::vector<long long> M(n + 1, 0);
    for (int j = 0; j < n; ++j) {
        long long include = sortedCourses[j].enrollments + M[p[j] + 1];
        long long exclude = M[j];
        M[j + 1] = include > exclude ? include : exclude;
    }
    // Reconstruct from the table; on a tie the course is NOT included.
    std::vector<Course> reversed;
    int j = n - 1;
    while (j >= 0) {
        long long include = sortedCourses[j].enrollments + M[p[j] + 1];
        if (include > M[j]) {
            reversed.push_back(sortedCourses[j]);
            j = p[j];
        } else {
            --j;
        }
    }
    return std::vector<Course>(reversed.rbegin(), reversed.rend());
}

std::vector<Course> selectCoursesDP(const std::vector<Course>& sortedCourses) {
    return selectByPredecessors(sortedCourses,
                                computePredecessorsScan(sortedCourses));
}

std::vector<Course> selectCoursesDPBinary(
    const std::vector<Course>& sortedCourses) {
    return selectByPredecessors(sortedCourses,
                                computePredecessorsBinary(sortedCourses));
}

// ================================ ROOMS ====================================

std::vector<std::vector<Course>> allocateClassrooms(
    const std::vector<Course>& courses) {
    std::vector<Course> sorted = courses;   // the input is not modified
    mergeSortByStart(sorted);

    std::vector<std::vector<Course>> rooms;
    std::vector<int> lastFinish;            // last finish time per room

    for (const auto& c : sorted) {
        int best = -1;
        for (std::size_t r = 0; r < rooms.size(); ++r) {
            if (lastFinish[r] <= c.start &&
                (best == -1 || lastFinish[r] < lastFinish[best])) {
                best = static_cast<int>(r);   // strict <: lowest index on ties
            }
        }
        if (best == -1) {                    // no free room: open a new one
            rooms.push_back(std::vector<Course>{c});
            lastFinish.push_back(c.finish);
        } else {
            rooms[best].push_back(c);
            lastFinish[best] = c.finish;
        }
    }
    return rooms;
}

// ========================== exhaustive checkers ============================

std::size_t optimalCountExhaustive(const std::vector<Course>& courses) {
    const std::size_t n = courses.size();
    std::size_t best = 0;
    for (unsigned mask = 0; mask < (1u << n); ++mask) {
        std::vector<Course> subset;
        for (std::size_t i = 0; i < n; ++i)
            if (mask & (1u << i)) subset.push_back(courses[i]);
        if (pairwiseCompatible(subset) && subset.size() > best)
            best = subset.size();
    }
    return best;
}

long long optimalEnrollmentsExhaustive(const std::vector<Course>& courses) {
    const std::size_t n = courses.size();
    long long best = 0;
    for (unsigned mask = 0; mask < (1u << n); ++mask) {
        std::vector<Course> subset;
        for (std::size_t i = 0; i < n; ++i)
            if (mask & (1u << i)) subset.push_back(courses[i]);
        if (pairwiseCompatible(subset)) {
            long long sum = totalEnrollments(subset);
            if (sum > best) best = sum;
        }
    }
    return best;
}

std::size_t minClassroomsExhaustive(const std::vector<Course>& courses) {
    if (courses.empty()) return 0;
    std::size_t best = courses.size() + 1;  // n rooms always works
    std::vector<std::vector<Course>> rooms;
    partitionSearch(courses, 0, rooms, best);
    return best;
}
