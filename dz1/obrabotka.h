#pragma once

#include <boost/thread.hpp>
#include <mutex>
#include <condition_variable>
#include <atomic>
#include <chrono>
#include <thread>
#include <string>
#include <vector>
#include <fstream>
#include <sstream>
#include <iostream>

using namespace std;

struct FileStats {
    string filename;
    int words;
    int chars;
    int lines;

    FileStats();
};

class FileProcessor {
private:
    vector<string> files;
    vector<FileStats> stats;

    mutex mtx;
    mutex coutMutex;
    condition_variable cv;
    atomic<int> processed;

    chrono::steady_clock::time_point start;
    chrono::steady_clock::time_point end;

    void processFile(int index);
    FileStats analyzeFile(const string& path);
    void writeResult(const FileStats& stat);

public:
    FileProcessor();
    ~FileProcessor();

    void addFile(const string& path);
    void processAll();
    void printStats();
};
