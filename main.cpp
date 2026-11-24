#include <iostream>
#include <vector>

#include "Algorithms.h"

namespace {

// Prints "<count> <sum>" followed by one line per course.
void printSelection(const std::vector<Course>& chosen) {
    std::cout << chosen.size() << ' ' << totalEnrollments(chosen) << '\n';
    for (const auto& c : chosen)
        std::cout << c.id << ' ' << c.start << ' ' << c.finish << ' '
                  << c.enrollments << '\n';
}

} // namespace

int main() {
    std::size_t n = 0;
    if (!(std::cin >> n)) n = 0;

    std::vector<Course> original(n);
    for (auto& c : original)
        std::cin >> c.id >> c.start >> c.finish >> c.enrollments;

    // COUNT and ENROLLMENTS work on a copy sorted by finish time.
    std::vector<Course> byFinish = original;
    mergeSort(byFinish);
    std::vector<Course> countResult = selectCourses(byFinish);
    std::vector<Course> enrollResult = selectCoursesDP(byFinish);

    // ROOMS receives the original list and sorts its own copy by start time.
    std::vector<std::vector<Course>> rooms = allocateClassrooms(original);

    std::cout << "COUNT\n";
    printSelection(countResult);

    std::cout << "ROOMS\n" << rooms.size() << '\n';
    for (std::size_t r = 0; r < rooms.size(); ++r)
        for (const auto& c : rooms[r])
            std::cout << (r + 1) << ' ' << c.id << ' ' << c.start << ' '
                      << c.finish << ' ' << c.enrollments << '\n';

    std::cout << "ENROLLMENTS\n";
    printSelection(enrollResult);
    return 0;
}
