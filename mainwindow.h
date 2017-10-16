#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>

namespace Ui {
class MainWindow;
}

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = 0);
    ~MainWindow();

private slots:
    void on_openImageButton_released();

    void on_openMkimage_released();

    void on_selectOutputDirectoryButton_released();

    void on_splitImageButton_released();

    void on_generateScriptButton_released();

    void on_mkimageScript_released();

    void on_makeOutputDirectory_released();

private:
    Ui::MainWindow *ui;
};

#endif // MAINWINDOW_H
