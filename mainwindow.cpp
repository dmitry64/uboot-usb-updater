#include "mainwindow.h"
#include "ui_mainwindow.h"
#include <QDebug>
#include <QDesktopServices>
#include <QDirIterator>
#include <QFileDialog>
#include <QProcess>

static const QString outputFilesSuffix = "part_";

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow)
{
    ui->setupUi(this);
}

MainWindow::~MainWindow()
{
    delete ui;
}

void MainWindow::on_openImageButton_released()
{
    ui->progressBar->setValue(0);
    QString fileName = QFileDialog::getOpenFileName(this, tr("Open Image"), "./", tr("Image Files (*.img *.bin *.iso);;Any Files (*)"));
    ui->inputImagePath->setText(fileName);
    QFileInfo info(fileName);
    qint64 size = info.size();
    ui->inputImageInfo->setText(QString("Size = %0\nBlocks = %2 (bs = 512)").arg(size).arg(size/512));
}

void MainWindow::on_openMkimage_released()
{
    ui->progressBar->setValue(0);
    QString fileName = QFileDialog::getOpenFileName(this, tr("Path to mkimage"), "./", tr("Any Files (*)"));
    ui->mkimagePath->setText(fileName);
}

void MainWindow::on_selectOutputDirectoryButton_released()
{
    ui->progressBar->setValue(0);
    QString path = QFileDialog::getExistingDirectory(this,tr("Output directory"),"./",QFileDialog::ShowDirsOnly);
    ui->outputPath->setText(path);
}

void MainWindow::on_splitImageButton_released()
{
    ui->progressBar->setValue(0);
    QString imagePath = ui->inputImagePath->text();
    QString outputPath = ui->outputPath->text();

    if(imagePath.size()==0 || outputPath.size()==0) {
        return;
    }
    qDebug() << "Splitting image:" << imagePath;
    ui->statusBar->showMessage("Splitting image...");
    ui->progressBar->setValue(30);

    QString splitCommand = "split --bytes=100M " + imagePath + " " + outputPath + "/" + outputFilesSuffix;
    qDebug() << "Splitting command:" <<splitCommand;
    ui->progressBar->setValue(33);

    QProcess process;
    process.start("sh");
    process.waitForStarted();
    ui->progressBar->setValue(38);
    process.write(splitCommand.toStdString().c_str());
    process.closeWriteChannel();
    ui->progressBar->setValue(45);
    process.waitForFinished(-1);
    ui->progressBar->setValue(70);
    process.write("sync");
    ui->progressBar->setValue(90);
    QByteArray output = process.readAllStandardOutput();
    process.close();


    qDebug() << "Splitting finished!";
    ui->statusBar->showMessage("Splitting finished!");
    ui->progressBar->setValue(100);
}

void MainWindow::on_generateScriptButton_released()
{
    ui->progressBar->setValue(0);
    QString outputPath = ui->outputPath->text();
    if(outputPath.size()==0) {
        return;
    }

    QString script;
    script += "echo \"STARTING\" \n";

    qDebug() << "Generating u-boot script...";
    ui->statusBar->showMessage("Generating u-boot script...");
    ui->progressBar->setValue(30);


    QDirIterator it(outputPath, QDir::Files | QDir::NoDotAndDotDot, QDirIterator::NoIteratorFlags);
    qint64 offset = 0;
    while (it.hasNext()) {
        QString currentFile = it.next();
        if(currentFile.contains(outputFilesSuffix)) {
            qDebug() << "Adding file:" << currentFile;

            QFileInfo info(currentFile);
            qint64 sizeBytes = info.size();
            qint64 sizeBlocks = sizeBytes / 512;

            QString sizeBlocksHex = QString::number(sizeBlocks, 16);


            qDebug() << "Size:" <<sizeBytes << " bytes" << sizeBlocks << " blocks" << "0x"+sizeBlocksHex << " blocks";

            script += "fatload ${bdev} ${binst}:${bpart} 0x90800000 "+info.fileName() + "\n";
            script += "echo \"Loading file "+info.fileName()+"\" \n";
            script += "mmc write 0x90800000 0x"+QString::number(offset, 16)+" 0x"+sizeBlocksHex+"\n";
            script += "echo \"File "+info.fileName()+" written to mmc\" \n";

            offset+=sizeBlocks;
        }
    }

    script += "echo \"SCRIPT FINISHED\" \n";
    script += "clrlogo \n";
    ui->scriptTextEdit->setPlainText(script);

    QFile outputFile(outputPath + "/updater_script.txt");
    outputFile.open(QIODevice::WriteOnly);
    outputFile.write(script.toStdString().c_str(),script.size());
    outputFile.close();

    qDebug() << "U-boot script generated!";
    ui->statusBar->showMessage("U-boot script generated!");
    ui->progressBar->setValue(100);
}

void MainWindow::on_mkimageScript_released()
{
    ui->progressBar->setValue(0);
    QString outputPath = ui->outputPath->text();
    QString mkimagePath = ui->mkimagePath->text();
    if(outputPath.size()==0 || mkimagePath.size()==0) {
        return;
    }

    QString command = mkimagePath + " -T script -C none -n 'Bootscript file' -d " + outputPath + "/updater_script.txt " + outputPath + "/bscript.img";
    qDebug() << "Command:" << command;

    QProcess process;
    process.start("sh");
    process.waitForStarted();
    ui->progressBar->setValue(20);
    process.write(command.toStdString().c_str());
    process.closeWriteChannel();
    ui->progressBar->setValue(30);
    process.waitForFinished(-1);
    ui->progressBar->setValue(40);
    process.write("sync");
    ui->progressBar->setValue(90);
    QByteArray output = process.readAllStandardOutput();
    process.close();
    qDebug() << output;

    qDebug() << "U-boot script image generated!";
    ui->statusBar->showMessage("U-boot script image generated!");
    ui->progressBar->setValue(100);
}

void MainWindow::on_makeOutputDirectory_released()
{
    ui->progressBar->setValue(0);
    QString outputPath = ui->outputPath->text();
    if(outputPath.size()==0) {
        return;
    }
    QDesktopServices::openUrl(outputPath);
}
