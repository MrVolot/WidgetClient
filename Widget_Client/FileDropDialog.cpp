#include "filedropdialog.h"

FileDropDialog::FileDropDialog(const QString &filePath, QWidget *parent) : QDialog(parent) {
    QFileInfo fileInfo(filePath);

    fileNameLabel = new QLabel(fileInfo.fileName(), this);
    fileSizeLabel = new QLabel(QString::number(fileInfo.size() / 1024.0, 'f', 2) + " KB", this);
    acceptButton = new QPushButton("Accept", this);
    rejectButton = new QPushButton("Reject", this);

    setupUI();

    connect(acceptButton, &QPushButton::clicked, this, &QDialog::accept);
    connect(rejectButton, &QPushButton::clicked, this, &QDialog::reject);
}

void FileDropDialog::setupUI() {
    QVBoxLayout *mainLayout = new QVBoxLayout(this);

    QHBoxLayout *buttonLayout = new QHBoxLayout();
    buttonLayout->addWidget(acceptButton);
    buttonLayout->addWidget(rejectButton);

    mainLayout->addWidget(fileNameLabel);
    mainLayout->addWidget(fileSizeLabel);
    mainLayout->addLayout(buttonLayout);

    setLayout(mainLayout);
}
