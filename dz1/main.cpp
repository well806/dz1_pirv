#include "obrabotka.h"
#include <cstdlib>

int main() {
    setlocale(LC_ALL, "RU");

    FileProcessor processor;

    processor.addFile("data1.txt");
    processor.addFile("data2.txt");
    processor.addFile("data3.txt");

    cout << "Обработка файлов..." << endl;

    processor.processAll();

    processor.printStats();

    return 0;
}
