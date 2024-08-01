#ifndef TRANSFORMIMAGESDIALOG_H
#define TRANSFORMIMAGESDIALOG_H

#include <QDialog>
#include <QStringList>

namespace Ui {
class TransformImagesDialog;
}

class TransformImagesDialog : public QDialog
{
    Q_OBJECT

public:
    explicit TransformImagesDialog(QStringList strList, QWidget *parent = 0);
    ~TransformImagesDialog();
    QVector<QString> checkedImages;
    double H;

private slots:
    void on_buttonBox_accepted();

private:
    Ui::TransformImagesDialog *ui;
};

#endif // TRANSFORMIMAGESDIALOG_H
