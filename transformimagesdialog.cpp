#include "transformimagesdialog.h"
#include "ui_transformimagesdialog.h"
#include <QComboBox>

TransformImagesDialog::TransformImagesDialog(QStringList strList, QWidget *parent) :
    QDialog(parent),
    ui(new Ui::TransformImagesDialog)
{
    ui->setupUi(this);
    ui->cbImage1->addItems(strList);
    ui->cbImage2->addItems(strList);
//    ui->listWidget->addItems(strList);
//    QListWidgetItem* item = 0;
//    for(int i = 0; i < ui->listWidget->count(); ++i){
//        item = ui->listWidget->item(i);
//        item->setFlags(item->flags() | Qt::ItemIsUserCheckable);
//        item->setCheckState(Qt::Unchecked);
//    }
//    ui->listWidget->item
}

TransformImagesDialog::~TransformImagesDialog()
{
    delete ui;
}

void TransformImagesDialog::on_buttonBox_accepted()
{
//    QListWidgetItem* item = 0;
//    for (int i = 0; i < ui->listWidget->count(); ++i){
//        item = ui->listWidget->item(i);
//        if (item->checkState() == Qt::Checked)
//            checkedImages.append(item->text());
//    }

//    QComboBox* a = new QComboBox();
    checkedImages.resize(2);
    checkedImages[0] = ui->cbImage1->currentText();
    checkedImages[1] = ui->cbImage2->currentText();

    H = ui->doubleSpinBox->value();
}
