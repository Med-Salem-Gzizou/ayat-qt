# ayat-qt

### _Simple tool for the search and study of the Qurʾanic text_

البرنامج أداة مبسطة للدراسة والبحث في النص القرآني ويمكن إستعماله في حفض القرآن.

البرنامج يحتوي على العديد من التفاسير والترجمات، كذالك يمكنه قراءة النص القرآني بأصوات العديد من القراء ويحتوي أيضا على بعض القراءات المترجمة.

* ayat-qt written in C++ using [Qt5](https://www.qt.io/).
* The Qur'anic text, the translations and the interpretations used here extracted from [Ayat Software](https://quran.ksu.edu.sa/ayat/?l=en).
* The recitations will be downloaded from [EveryAyah Quran Files](https://www.everyayah.com/index.html).

## Features

* English and Arabic User Interface.
* Search for Verses and export to CSV file.
* 80 Audio Recitations (Include audio translations).
* 39 Translations and interpretations (Includes translations in different languages).

## Compiling & Running (debian)

 1. Install the required libraries and tools.
```sh
sudo apt install -y qtbase5-dev
sudo apt install -y qtmultimedia5-dev
sudo apt install -y qttools5-dev-tools
```

 2. Compile the code.
```sh
cd ayat-qt/
qmake ./
make
lrelease ayat-qt_ar_001.ts
```

 3. Run the software
```sh
./ayat-qt
```

## Screenshots

![MainWindow_Screenshot](https://github.com/Med-Salem-Gzizou/ayat-qt/blob/main/images/MainWindow_Screenshot.png?raw=true)

![DownloadsWindow_Screenshot](https://github.com/Med-Salem-Gzizou/ayat-qt/blob/main/images/DownloadsWindow_Screenshot.png?raw=true)

