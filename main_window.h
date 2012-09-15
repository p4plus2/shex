#ifndef MAIN_WINDOW_H
#define MAIN_WINDOW_H

#include <QtGui/QMainWindow>

class main_window : public QMainWindow
{
    Q_OBJECT

public:
    main_window(QWidget *parent = 0);
    ~main_window();
};

#endif // MAIN_WINDOW_H
