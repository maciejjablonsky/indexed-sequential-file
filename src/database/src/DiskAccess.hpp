#pragma once

struct DiskAccess {
    int reads;
    int writes;
    inline void Write() { ++writes; }
    inline void Read() { ++reads; }
    inline int CountWrites() const { return writes; }
    inline int CountReads() const { return reads; }
    inline void Reset() {
        writes = 0;
        reads = 0;
    }
    inline DiskAccess &operator+=(const DiskAccess &rhs) {
        reads += rhs.reads;
        writes += rhs.writes;
        return *this;
    }
    inline DiskAccess operator+(const DiskAccess &rhs) {
        DiskAccess tmp = *this;
        tmp += rhs;
        return tmp;
    }
};
