#ifndef COURSE_H
#define COURSE_H

#include <string>

// A single course meeting on a given day.
// Times are minutes from the start of the day; [start, finish) is the meeting.
struct Course {
    std::string id;          // unique English identifier, no spaces
    int start;               // start time in minutes
    int finish;              // finish time in minutes (start < finish)
    long long enrollments;   // positive number of enrolled students (weight)
};

#endif // COURSE_H
