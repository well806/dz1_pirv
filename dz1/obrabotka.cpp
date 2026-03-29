#include "obrabotka.h"
#include <boost/bind.hpp>

FileStats::FileStats() : words(0), chars(0), lines(0) {}

FileProcessor::FileProcessor() {
    processed = 0;
}

FileProcessor::~FileProcessor() {
}

void FileProcessor::addFile(const string& path) {
    files.push_back(path);
    stats.push_back(FileStats());
}

FileStats FileProcessor::analyzeFile(const string& path) {
    FileStats stat;
    stat.filename = path;

    ifstream file(path);
    if (!file.is_open()) {
        lock_guard<mutex> lock(coutMutex);
        cerr << "Ошибка: не удалось открыть файл " << path << endl;
        return stat;
    }

    string line;
    string content;

    while (getline(file, line)) {
        stat.lines++;
        content += line + " ";
    }

    file.close();

    stat.chars = content.length();

    istringstream iss(content);
    string word;
    while (iss >> word) {
        stat.words++;
    }

    return stat;
}

void FileProcessor::writeResult(const FileStats& stat) {
    string outputName = "processed_" + stat.filename;

    ofstream outFile(outputName);
    if (outFile.is_open()) {
        outFile << "Файл: " << stat.filename << endl;
        outFile << "Слов: " << stat.words << endl;
        outFile << "Символов: " << stat.chars << endl;
        outFile << "Строк: " << stat.lines << endl;
        outFile.close();
    }
}

void FileProcessor::processFile(int index) {
    FileStats stat = analyzeFile(files[index]);

    {
        lock_guard<mutex> lock(mtx);
        stats[index] = stat;
    }

    writeResult(stat);

    int current = processed.fetch_add(1) + 1;

    {
        lock_guard<mutex> lock(coutMutex);
        cout << "Поток " << this_thread::get_id() << " начал обработку: " << files[index] << endl;
        cout << "Поток " << this_thread::get_id() << " завершил файл " << files[index]
             << " (обработано " << current << " из " << files.size() << ")" << endl;
        cout << "Поток " << this_thread::get_id() << " отправил уведомление главному потоку" << endl;
        cout << "----------------------------------------" << endl;
    }

    cv.notify_all();
}

void FileProcessor::processAll() {
    start = chrono::steady_clock::now();

    {
        lock_guard<mutex> lock(coutMutex);
        cout << "\n=== Запуск " << files.size() << " потоков для обработки файлов ===" << endl;
    }

    vector<boost::thread> threads;

    for (size_t i = 0; i < files.size(); i++) {
        threads.push_back(boost::thread(&FileProcessor::processFile, this, i));
    }

    {
        lock_guard<mutex> lock(coutMutex);
        cout << "\nГлавный поток ожидает завершения всех рабочих потоков..." << endl;
    }

    unique_lock<mutex> lock(mtx);
    cv.wait(lock, [this]() {
        return processed >= (int)files.size();
    });

    {
        lock_guard<mutex> lock(coutMutex);
        cout << "Главный поток получил уведомление о завершении всех работ!" << endl;
    }

    for (size_t i = 0; i < threads.size(); i++) {
        threads[i].join();
    }

    end = chrono::steady_clock::now();

    {
        lock_guard<mutex> lock(coutMutex);
        cout << "Все потоки успешно завершены и присоединены." << endl;
    }
}

void FileProcessor::printStats() {
    lock_guard<mutex> lock(coutMutex);
    cout << "\n===== Статистика обработки =====" << endl;
    cout << "Всего обработано файлов: " << processed << endl;

    auto duration = chrono::duration_cast<chrono::milliseconds>(end - start);
    cout << "Общее время: " << duration.count() / 1000.0 << " секунд" << endl;

    cout << "\nДетали по файлам:" << endl;

    for (size_t i = 0; i < stats.size(); i++) {
        cout << "\nФайл: " << stats[i].filename << endl;
        cout << "  Слов: " << stats[i].words << endl;
        cout << "  Символов: " << stats[i].chars << endl;
        cout << "  Строк: " << stats[i].lines << endl;
    }
}
