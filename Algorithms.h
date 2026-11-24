#ifndef ALGORITHMS_H
#define ALGORITHMS_H

#include <string>
#include <vector>
#include <cstddef>

#include "Course.h"

// ---------------------------------------------------------------------------
// Required interface
// ---------------------------------------------------------------------------

// Sorts by finish time, then start time, then id (all ascending).
// Implemented with a hand-written recursive merge sort; only reorders records.
void mergeSort(std::vector<Course>& courses);

// COUNT: greedy earliest-finish selection for a single classroom.
// Input must be sorted by mergeSort. Returns the selected original records.
std::vector<Course> selectCourses(
    const std::vector<Course>& sortedCourses);

// ENROLLMENTS: weighted interval scheduling by dynamic programming.
// Input must be sorted by mergeSort. Predecessors are found by a backward scan.
std::vector<Course> selectCoursesDP(
    const std::vector<Course>& sortedCourses);

// Test-only brute force (at most 5 courses, any order).
std::size_t optimalCountExhaustive(
    const std::vector<Course>& courses);

// ROOMS: greedy allocation of all courses to a minimum number of classrooms.
// Input in any order; the input is not modified.
std::vector<std::vector<Course>> allocateClassrooms(
    const std::vector<Course>& courses);

// Test-only brute force (at most 5 courses, any order).
std::size_t minClassroomsExhaustive(
    const std::vector<Course>& courses);

// Test-only brute force (at most 5 courses, any order).
long long optimalEnrollmentsExhaustive(
    const std::vector<Course>& courses);

// ---------------------------------------------------------------------------
// Optional extension and helper functions
// ---------------------------------------------------------------------------

// Same as selectCoursesDP, but predecessors are found by binary search.
std::vector<Course> selectCoursesDPBinary(
    const std::vector<Course>& sortedCourses);

// Sorts by start time, then finish time, then id (used by allocateClassrooms).
void mergeSortByStart(std::vector<Course>& courses);

// p[j] = largest index i < j with sorted[i].finish <= sorted[j].start, or -1.
std::vector<int> computePredecessorsScan(
    const std::vector<Course>& sortedCourses);
std::vector<int> computePredecessorsBinary(
    const std::vector<Course>& sortedCourses);

// Shared DP: fills the table using the given predecessors and reconstructs
// the chosen courses. On a tie between "include" and "exclude", excludes.
std::vector<Course> selectByPredecessors(
    const std::vector<Course>& sortedCourses,
    const std::vector<int>& p);

// True if two courses overlap in time (touching endpoints do not overlap).
bool coursesOverlap(const Course& a, const Course& b);

// Sum of enrollments of a list of courses.
long long totalEnrollments(const std::vector<Course>& courses);

#endif // ALGORITHMS_H
