#ifndef FILEDROPDIALOG_H
#define FILEDROPDIALOG_H

#include <QDialog>
#include <QLabel>
#include <QPushButton>
#include <QHBoxLayout>
#include <QVBoxLayout>
#include <QFileInfo>

class FileDropDialog : public QDialog {
    Q_OBJECT

public:
    explicit FileDropDialog(const QString &filePath, QWidget *parent = nullptr);

private:
    QLabel *fileNameLabel;
    QLabel *fileSizeLabel;
    QPushButton *acceptButton;
    QPushButton *rejectButton;

    void setupUI();
};

#endif // FILEDROPDIALOG_H
