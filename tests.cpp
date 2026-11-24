#define DOCTEST_CONFIG_IMPLEMENT_WITH_MAIN
#include "doctest.h"
#include "Algorithms.h"

#include <cstdint>
#include <set>
#include <string>
#include <vector>

// ===========================================================================
// Test helpers
// ===========================================================================

namespace {

Course C(const std::string& id, int s, int f, long long e) {
    return Course{id, s, f, e};
}

bool sameCourse(const Course& a, const Course& b) {
    return a.id == b.id && a.start == b.start && a.finish == b.finish &&
           a.enrollments == b.enrollments;
}

bool sameList(const std::vector<Course>& a, const std::vector<Course>& b) {
    if (a.size() != b.size()) return false;
    for (std::size_t i = 0; i < a.size(); ++i)
        if (!sameCourse(a[i], b[i])) return false;
    return true;
}

// True if c appears in input with exactly the same fields.
bool takenFromInput(const Course& c, const std::vector<Course>& input) {
    for (const auto& x : input)
        if (sameCourse(x, c)) return true;
    return false;
}

std::vector<Course> sortedByFinish(std::vector<Course> v) {
    mergeSort(v);
    return v;
}

// Single-room selection is legal: records come unchanged from the input,
// no duplicate ids, listed in time order, and no two courses overlap.
void checkSingleRoomSelection(const std::vector<Course>& input,
                              const std::vector<Course>& chosen) {
    std::set<std::string> ids;
    for (std::size_t i = 0; i < chosen.size(); ++i) {
        CHECK(takenFromInput(chosen[i], input));
        CHECK(ids.insert(chosen[i].id).second);
        if (i > 0) CHECK(chosen[i - 1].finish <= chosen[i].start);
        for (std::size_t k = i + 1; k < chosen.size(); ++k)
            CHECK_FALSE(coursesOverlap(chosen[i], chosen[k]));
    }
}

// Room allocation is legal: every input course exactly once, unchanged,
// no empty room, each room in time order with no overlaps.
void checkRooms(const std::vector<Course>& input,
                const std::vector<std::vector<Course>>& rooms) {
    std::size_t total = 0;
    std::set<std::string> ids;
    for (const auto& room : rooms) {
        CHECK_FALSE(room.empty());
        for (std::size_t i = 0; i < room.size(); ++i) {
            CHECK(takenFromInput(room[i], input));
            CHECK(ids.insert(room[i].id).second);
            if (i > 0) CHECK(room[i - 1].finish <= room[i].start);
            for (std::size_t k = i + 1; k < room.size(); ++k)
                CHECK_FALSE(coursesOverlap(room[i], room[k]));
        }
        total += room.size();
    }
    CHECK(total == input.size());
}

// Maximum number of courses running at the same moment (independent of the
// greedy allocation): a lower bound that the optimal number of rooms meets.
std::size_t maxDepth(const std::vector<Course>& courses) {
    std::size_t best = 0;
    for (const auto& c : courses) {
        std::size_t depth = 0;
        for (const auto& d : courses)
            if (d.start <= c.start && c.start < d.finish) ++depth;
        if (depth > best) best = depth;
    }
    return best;
}

// Deterministic generator (same sequence on every platform/compiler).
struct Lcg {
    std::uint64_t state;
    explicit Lcg(std::uint64_t seed) : state(seed) {}
    int next(int lo, int hi) {                 // inclusive range
        state = state * 6364136223846793005ULL + 1442695040888963407ULL;
        std::uint64_t x = state >> 33;
        return lo + static_cast<int>(x % static_cast<std::uint64_t>(hi - lo + 1));
    }
};

std::vector<Course> randomCourses(Lcg& rng, int n, int maxTime, int maxEnroll) {
    std::vector<Course> v;
    for (int i = 0; i < n; ++i) {
        int s = rng.next(0, maxTime - 1);
        int f = rng.next(s + 1, maxTime);
        v.push_back(C("C" + std::to_string(i), s, f, rng.next(1, maxEnroll)));
    }
    return v;
}

// Runs every check that does not need a brute force.
void checkAllValid(const std::vector<Course>& input) {
    std::vector<Course> sorted = sortedByFinish(input);
    std::vector<Course> countRes = selectCourses(sorted);
    std::vector<Course> dpRes = selectCoursesDP(sorted);
    std::vector<Course> dpBin = selectCoursesDPBinary(sorted);
    auto rooms = allocateClassrooms(input);

    checkSingleRoomSelection(input, countRes);
    checkSingleRoomSelection(input, dpRes);
    checkRooms(input, rooms);
    CHECK(rooms.size() == maxDepth(input));
    CHECK(computePredecessorsScan(sorted) == computePredecessorsBinary(sorted));
    CHECK(sameList(dpRes, dpBin));
    // The DP optimum is at least the weight of the greedy COUNT solution.
    CHECK(totalEnrollments(dpRes) >= totalEnrollments(countRes));
}

// Compares all three results with the exhaustive solutions (n <= 5).
void checkAgainstExhaustive(const std::vector<Course>& input) {
    std::vector<Course> sorted = sortedByFinish(input);
    CHECK(selectCourses(sorted).size() == optimalCountExhaustive(input));
    CHECK(totalEnrollments(selectCoursesDP(sorted)) ==
          optimalEnrollmentsExhaustive(input));
    CHECK(totalEnrollments(selectCoursesDPBinary(sorted)) ==
          optimalEnrollmentsExhaustive(input));
    CHECK(allocateClassrooms(input).size() == minClassroomsExhaustive(input));
}

} // namespace

// ===========================================================================
// Merge sort
// ===========================================================================

TEST_CASE("mergeSort orders by finish, then start, then id") {
    std::vector<Course> v = {C("Z", 5, 10, 1), C("B", 0, 10, 1),
                             C("A", 0, 10, 1), C("Q", 1, 3, 1),
                             C("M", 2, 20, 1)};
    mergeSort(v);
    REQUIRE(v.size() == 5);
    CHECK(v[0].id == "Q");
    CHECK(v[1].id == "A");   // same finish and start as B: id breaks tie
    CHECK(v[2].id == "B");
    CHECK(v[3].id == "Z");   // same finish, later start
    CHECK(v[4].id == "M");
}

TEST_CASE("mergeSortByStart orders by start, then finish, then id") {
    std::vector<Course> v = {C("D", 5, 9, 1), C("B", 0, 10, 1),
                             C("A", 0, 10, 1), C("C", 0, 4, 1)};
    mergeSortByStart(v);
    REQUIRE(v.size() == 4);
    CHECK(v[0].id == "C");
    CHECK(v[1].id == "A");
    CHECK(v[2].id == "B");
    CHECK(v[3].id == "D");
}

TEST_CASE("mergeSort keeps exactly the same records (permutation)") {
    Lcg rng(7);
    for (int t = 0; t < 200; ++t) {
        std::vector<Course> v = randomCourses(rng, rng.next(0, 40), 100, 50);
        std::vector<Course> s = v;
        mergeSort(s);
        REQUIRE(s.size() == v.size());
        for (const auto& c : v) CHECK(takenFromInput(c, s));
        for (std::size_t i = 1; i < s.size(); ++i) {
            bool ordered = s[i - 1].finish < s[i].finish ||
                (s[i - 1].finish == s[i].finish &&
                 (s[i - 1].start < s[i].start ||
                  (s[i - 1].start == s[i].start && s[i - 1].id < s[i].id)));
            CHECK(ordered);
        }
    }
}

TEST_CASE("mergeSort on empty and single-element vectors") {
    std::vector<Course> empty;
    mergeSort(empty);
    CHECK(empty.empty());
    std::vector<Course> one = {C("Solo", 1, 2, 3)};
    mergeSort(one);
    REQUIRE(one.size() == 1);
    CHECK(sameCourse(one[0], C("Solo", 1, 2, 3)));
}

// ===========================================================================
// The three examples from the assignment
// ===========================================================================

TEST_CASE("assignment sample") {
    std::vector<Course> in = {C("Algorithms", 540, 600, 4),
                              C("Databases", 600, 660, 4),
                              C("Operating_Systems", 540, 660, 11)};
    std::vector<Course> s = sortedByFinish(in);

    std::vector<Course> cnt = selectCourses(s);
    CHECK(sameList(cnt, {C("Algorithms", 540, 600, 4),
                         C("Databases", 600, 660, 4)}));
    CHECK(totalEnrollments(cnt) == 8);

    auto rooms = allocateClassrooms(in);
    REQUIRE(rooms.size() == 2);
    CHECK(sameList(rooms[0], {C("Algorithms", 540, 600, 4),
                              C("Databases", 600, 660, 4)}));
    CHECK(sameList(rooms[1], {C("Operating_Systems", 540, 660, 11)}));

    std::vector<Course> dp = selectCoursesDP(s);
    CHECK(sameList(dp, {C("Operating_Systems", 540, 660, 11)}));
    CHECK(totalEnrollments(dp) == 11);
    checkAgainstExhaustive(in);
}

TEST_CASE("assignment empty input") {
    std::vector<Course> in;
    CHECK(selectCourses(in).empty());
    CHECK(selectCoursesDP(in).empty());
    CHECK(allocateClassrooms(in).empty());
    CHECK(optimalCountExhaustive(in) == 0);
    CHECK(minClassroomsExhaustive(in) == 0);
    CHECK(optimalEnrollmentsExhaustive(in) == 0);
}

TEST_CASE("assignment ties in times and enrollments") {
    std::vector<Course> in = {C("Databases", 540, 600, 5),
                              C("Algorithms", 540, 600, 5),
                              C("Operating_Systems", 600, 660, 2)};
    std::vector<Course> s = sortedByFinish(in);
    std::vector<Course> expected = {C("Algorithms", 540, 600, 5),
                                    C("Operating_Systems", 600, 660, 2)};
    CHECK(sameList(selectCourses(s), expected));
    CHECK(sameList(selectCoursesDP(s), expected));

    auto rooms = allocateClassrooms(in);
    REQUIRE(rooms.size() == 2);
    CHECK(sameList(rooms[0], expected));
    CHECK(sameList(rooms[1], {C("Databases", 540, 600, 5)}));
    checkAgainstExhaustive(in);
}

// ===========================================================================
// Edge cases (each targets a specific possible mistake)
// ===========================================================================

TEST_CASE("single course is chosen everywhere") {
    std::vector<Course> in = {C("Only", 100, 200, 9)};
    std::vector<Course> s = sortedByFinish(in);
    CHECK(sameList(selectCourses(s), in));
    CHECK(sameList(selectCoursesDP(s), in));
    auto rooms = allocateClassrooms(in);
    REQUIRE(rooms.size() == 1);
    CHECK(sameList(rooms[0], in));
}

TEST_CASE("touching endpoints are compatible (catches < vs <=)") {
    std::vector<Course> in = {C("A", 0, 10, 1), C("B", 10, 20, 1),
                              C("C", 20, 30, 1)};
    std::vector<Course> s = sortedByFinish(in);
    CHECK(selectCourses(s).size() == 3);
    CHECK(totalEnrollments(selectCoursesDP(s)) == 3);
    CHECK(allocateClassrooms(in).size() == 1);
    const std::vector<int> expectedP = {-1, 0, 1};
    CHECK(computePredecessorsScan(s) == expectedP);
    checkAgainstExhaustive(in);
}

TEST_CASE("all courses overlap") {
    std::vector<Course> in = {C("A", 0, 100, 3), C("B", 10, 90, 7),
                              C("C", 20, 80, 5), C("D", 30, 70, 1)};
    std::vector<Course> s = sortedByFinish(in);
    CHECK(selectCourses(s).size() == 1);
    std::vector<Course> dp = selectCoursesDP(s);
    REQUIRE(dp.size() == 1);
    CHECK(dp[0].id == "B");
    CHECK(allocateClassrooms(in).size() == 4);
    checkAgainstExhaustive(in);
}

TEST_CASE("identical times with different ids") {
    std::vector<Course> in = {C("Gamma", 0, 10, 2), C("Alpha", 0, 10, 2),
                              C("Beta", 0, 10, 2)};
    std::vector<Course> s = sortedByFinish(in);
    std::vector<Course> cnt = selectCourses(s);
    REQUIRE(cnt.size() == 1);
    CHECK(cnt[0].id == "Alpha");
    auto rooms = allocateClassrooms(in);
    REQUIRE(rooms.size() == 3);
    CHECK(rooms[0][0].id == "Alpha");
    CHECK(rooms[1][0].id == "Beta");
    CHECK(rooms[2][0].id == "Gamma");
    checkAgainstExhaustive(in);
}

TEST_CASE("unsorted input and input is not modified") {
    std::vector<Course> in = {C("Late", 50, 60, 1), C("Early", 0, 10, 1),
                              C("Mid", 20, 30, 1)};
    std::vector<Course> copy = in;
    auto rooms = allocateClassrooms(in);
    CHECK(sameList(in, copy));               // allocateClassrooms keeps input
    REQUIRE(rooms.size() == 1);
    CHECK(sameList(rooms[0], {C("Early", 0, 10, 1), C("Mid", 20, 30, 1),
                              C("Late", 50, 60, 1)}));
    std::vector<Course> s = sortedByFinish(in);
    std::vector<Course> sCopy = s;
    selectCourses(s);
    selectCoursesDP(s);
    CHECK(sameList(s, sCopy));               // selections keep their input
    checkAgainstExhaustive(in);
}

TEST_CASE("max count and max enrollments differ") {
    std::vector<Course> in = {C("Short1", 0, 10, 1), C("Short2", 10, 20, 1),
                              C("Short3", 20, 30, 1), C("Long", 0, 30, 100)};
    std::vector<Course> s = sortedByFinish(in);
    CHECK(selectCourses(s).size() == 3);
    CHECK(totalEnrollments(selectCourses(s)) == 3);
    std::vector<Course> dp = selectCoursesDP(s);
    REQUIRE(dp.size() == 1);
    CHECK(dp[0].id == "Long");
    checkAgainstExhaustive(in);
}

TEST_CASE("DP tie rule: equal totals exclude the current course") {
    // Sorted by finish (then start): A(0-10,5), C(0-20,10), B(10-20,5).
    // Both {A,B} and {C} give 10. Reconstruction starts at B: including B
    // gives 5 + M[A] = 10, equal to excluding it, so B is excluded. Then C
    // (10 > 5) is included. Expected answer: {C}.
    std::vector<Course> in = {C("C", 0, 20, 10), C("A", 0, 10, 5),
                              C("B", 10, 20, 5)};
    std::vector<Course> s = sortedByFinish(in);
    REQUIRE(s.size() == 3);
    CHECK(s[1].id == "C");
    std::vector<Course> dp = selectCoursesDP(s);
    CHECK(sameList(dp, {C("C", 0, 20, 10)}));
    CHECK(totalEnrollments(dp) == 10);
    checkAgainstExhaustive(in);
}

TEST_CASE("ROOMS reuses a freed room with the earliest last finish") {
    // R1 ends at 10, R2 ends at 5; X starts at 12: both free, R2 ends earlier.
    std::vector<Course> in = {C("R1", 0, 10, 1), C("R2", 1, 5, 1),
                              C("X", 12, 20, 1)};
    auto rooms = allocateClassrooms(in);
    REQUIRE(rooms.size() == 2);
    CHECK(sameList(rooms[0], {C("R1", 0, 10, 1)}));
    CHECK(sameList(rooms[1], {C("R2", 1, 5, 1), C("X", 12, 20, 1)}));
    checkAgainstExhaustive(in);
}

TEST_CASE("ROOMS opens a new room only when no room is free") {
    std::vector<Course> in = {C("A", 0, 10, 1), C("B", 5, 15, 1),
                              C("C", 10, 20, 1), C("D", 12, 30, 1)};
    // A -> room 1; B overlaps A -> room 2; C starts when A ends -> room 1;
    // D at 12: room 1 busy until 20, room 2 until 15 -> new room 3.
    auto rooms = allocateClassrooms(in);
    REQUIRE(rooms.size() == 3);
    CHECK(sameList(rooms[0], {C("A", 0, 10, 1), C("C", 10, 20, 1)}));
    CHECK(sameList(rooms[1], {C("B", 5, 15, 1)}));
    CHECK(sameList(rooms[2], {C("D", 12, 30, 1)}));
    checkRooms(in, rooms);
    checkAgainstExhaustive(in);
}

// ===========================================================================
// Predecessor bug from the critique task
// ===========================================================================

TEST_CASE("p(j) must not only look at course j-1") {
    // Sorted by finish: Algorithms(600), Databases(660), Operating_Systems(720).
    // For Operating_Systems (start 630) the course j-1 = Databases overlaps,
    // but Algorithms (finish 600) is compatible, so p = 0, not -1.
    // The buggy rule would give p = {-1,-1,-1} and an optimum of 10 instead of 15.
    std::vector<Course> in = {C("Algorithms", 540, 600, 5),
                              C("Databases", 570, 660, 1),
                              C("Operating_Systems", 630, 720, 10)};
    std::vector<Course> s = sortedByFinish(in);
    const std::vector<int> correctP = {-1, -1, 0};
    const std::vector<int> buggyP = {-1, -1, -1};
    CHECK(computePredecessorsScan(s) == correctP);
    CHECK(computePredecessorsScan(s) != buggyP);
    CHECK(computePredecessorsBinary(s) == correctP);
    std::vector<Course> dp = selectCoursesDP(s);
    CHECK(totalEnrollments(dp) == 15);
    CHECK(sameList(dp, {C("Algorithms", 540, 600, 5),
                        C("Operating_Systems", 630, 720, 10)}));
    CHECK(optimalEnrollmentsExhaustive(in) == 15);
}

// ===========================================================================
// Exhaustive checkers sanity
// ===========================================================================

TEST_CASE("exhaustive checkers on hand-computed inputs") {
    std::vector<Course> in = {C("A", 0, 4, 2), C("B", 3, 6, 3),
                              C("C", 5, 9, 4), C("D", 8, 12, 1),
                              C("E", 0, 12, 9)};
    CHECK(optimalCountExhaustive(in) == 2);           // e.g. A,C or B,D
    CHECK(optimalEnrollmentsExhaustive(in) == 9);     // E alone, or A+C = 6
    CHECK(minClassroomsExhaustive(in) == 3);          // depth at time 3 is 3
}

// ===========================================================================
// Randomized tests (seed documented for reproduction)
// ===========================================================================

TEST_CASE("random small inputs match exhaustive solutions (seed 20261120)") {
    Lcg rng(20261120);
    for (int t = 0; t < 3000; ++t) {
        int n = rng.next(0, 5);
        // Small time range (0..20) forces many touching and equal endpoints.
        std::vector<Course> in = randomCourses(rng, n, 20, 6);
        checkAllValid(in);
        checkAgainstExhaustive(in);
    }
}

TEST_CASE("random large inputs are valid (seed 424242)") {
    Lcg rng(424242);
    for (int t = 0; t < 300; ++t) {
        int n = rng.next(6, 200);
        std::vector<Course> in = randomCourses(rng, n, 1440, 1000);
        checkAllValid(in);
    }
}
