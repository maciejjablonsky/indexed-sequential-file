#pragma once

struct DiskAccess {
    int reads;
    int writes;
    inline void Write() { ++writes; }
    inline void Read() { ++reads; }
    inline int CountWrites() const { return writes; }
    inline int CountReads() const { return reads; }
    inline void Reset() { writes = reads = 0; }
};
