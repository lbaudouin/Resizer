#ifndef QPROGRESSBARDIALOG_H
#define QPROGRESSBARDIALOG_H

#include <QDialog>
#include <QProgressBar>
#include <QVBoxLayout>
#include <QLabel>
#include <QDialogButtonBox>

#include <QNetworkReply>

class QProgressBarDialog : public QDialog
{
    Q_OBJECT
public:
    QProgressBarDialog(){
        this->setWindowTitle(tr("Download"));
        QVBoxLayout *vlayout = new QVBoxLayout(this);
        QLabel *label = new QLabel(tr("Please wait..."));
        vlayout->addWidget(label,0);
        progress = new QProgressBar;
        progress->setMinimum(0);
        progress->setMaximum(0);
        progress->setValue(0);
        vlayout->addWidget(progress);

        QDialogButtonBox *buttons = new QDialogButtonBox(QDialogButtonBox::Cancel);
        connect(buttons,SIGNAL(rejected()),this,SLOT(pressCancel()));
        vlayout->addWidget(buttons);
    }

    inline void setValue(int value) {progress->setValue(value); if(value>=progress->maximum()) this->accept();}
    inline void setMinimum(int minimum) {progress->setMinimum(minimum);}
    inline void setMaximum(int maximum) {progress->setMaximum(maximum);}
    inline void setFormat(QString format) {progress->setFormat(format);}

    inline void setReply(QNetworkReply* reply) {reply_ = reply;}
    inline QNetworkReply* getReply() {return reply_;}

private:
    QProgressBar *progress;
    QNetworkReply *reply_;

private slots:
    void pressCancel() {emit cancel(); this->reject();}

signals:
    void cancel();
};

#endif // QPROGRESSBARDIALOG_H
