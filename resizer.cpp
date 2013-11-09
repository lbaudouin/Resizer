#include "resizer.h"
#include "ui_resizer.h"

Resizer::Resizer(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::Resizer)
{
    ui->setupUi(this);

    diag_ = new QProgressDialog(tr("Loading"),tr("Cancel"),0,0,this);
    diag_->setValue(0);
    diag_->setMinimumDuration(50);

    QStringList listSize;
    listSize << "320" << "480" << "640" << "720" << "800" << "1024" << "1280" << "2048" << "4096";
    ui->comboPixels->addItems( listSize );
    ui->comboPixels->setCurrentIndex(5);
    QStringList listRatio;
    listRatio << "10" << "20" << "30" << "33" << "40" << "50" << "60" << "66" << "70" << "80" << "90" << "125" << "150" << "200" << "250" << "300";
    ui->comboRatio->addItems( listRatio );
    ui->comboRatio->setCurrentIndex(3);

    QIntValidator *validatorNotNull = new QIntValidator(this);
    validatorNotNull->setRange(8,32768);
    ui->comboPixels->lineEdit()->setValidator(validatorNotNull);


    QIntValidator *validator = new QIntValidator(this);
    validator->setRange(0,32768);
    ui->horizontalLineEdit->setValidator(validator);
    ui->verticalLineEdit->setValidator(validator);



    connect(ui->buttonOpenFolder,SIGNAL(pressed()),this,SLOT(pressOpenFolder()));
    connect(ui->buttonOpenFiles,SIGNAL(pressed()),this,SLOT(pressOpenFiles()));
    connect(ui->actionAdd_folder,SIGNAL(triggered()),this,SLOT(pressOpenFolder()));
    connect(ui->actionAdd_files,SIGNAL(triggered()),this,SLOT(pressOpenFiles()));

    connect(ui->groupRatio,SIGNAL(clicked(bool)),this,SLOT(setRatioMode(bool)));
    connect(ui->groupSize,SIGNAL(clicked(bool)),this,SLOT(setSizeMode(bool)));

    connect(ui->buttonLogo,SIGNAL(clicked()),this,SLOT(openLogo()));

    ui->buttonOpenFiles->setText("");
    ui->buttonOpenFolder->setText("");
    ui->buttonOpenFiles->setIcon(QIcon(":images/pictures"));
    ui->buttonOpenFolder->setIcon(QIcon(":images/folder"));

    ui->actionAdd_files->setIcon(QIcon(":images/pictures"));
    ui->actionAdd_folder->setIcon(QIcon(":images/folder"));

    connect(ui->buttonBox,SIGNAL(rejected()),this,SLOT(close()));
    connect(ui->buttonBox,SIGNAL(accepted()),this,SLOT(resizeAll()));

    readSettings();

    setLogo(logoPath);

    connect(ui->actionAbout,SIGNAL(triggered()),this,SLOT(pressAbout()));
}

Resizer::~Resizer()
{
    writeSettings();
    delete ui;
}

void Resizer::writeSettings()
{
    QSettings settings(qAppName(), "config");

    settings.setValue("options/keep_exif",ui->checkExif->isChecked());
    settings.setValue("options/no_resize",ui->checkNotResize->isChecked());
    settings.setValue("options/rotate",ui->checkRotate->isChecked());

    settings.setValue("small/mode",ui->groupRatio->isChecked()?"ratio":"size");
    settings.setValue("small/ratio",ui->comboRatio->currentText());
    settings.setValue("small/pixels",ui->comboPixels->currentText());

    settings.setValue("logo/attach",ui->selector->position());
    settings.setValue("logo/shift-x",ui->horizontalLineEdit->text().toInt());
    settings.setValue("logo/shift-y",ui->verticalLineEdit->text().toInt());
    settings.setValue("logo/path",logoPath);
}

void Resizer::readSettings()
{
    QSettings settings(qAppName(), "config");

    ui->checkExif->setChecked( settings.value("options/keep_exif",true).toBool() );
    ui->checkNotResize->setChecked( settings.value("options/no_resize",false).toBool() );
    ui->checkRotate->setChecked( settings.value("options/rotate",true).toBool() );


    setSizeMode( settings.value("small/mode","size")=="size"?true:false );
    ui->comboRatio->setCurrentIndex(ui->comboRatio->findText(settings.value("small/ratio",33).toString()));
    int pixelsIndex = ui->comboPixels->findText(settings.value("small/pixels",1024).toString());
    if(pixelsIndex>=0)
        ui->comboPixels->setCurrentIndex(pixelsIndex);
    else
        ui->comboPixels->lineEdit()->setText(settings.value("small/pixels",1024).toString());

    ui->selector->setPosition( static_cast<PositionSelector::POSITION>(settings.value("logo/attach",3).toInt()) );
    ui->horizontalLineEdit->setText( settings.value("logo/shift-x",25).toString() );
    ui->verticalLineEdit->setText( settings.value("logo/shift-y",25).toString() );
    logoPath = settings.value("logo/path",QDesktopServices::storageLocation(QDesktopServices::PicturesLocation)  + QDir::separator() + "logo.png").toString();
}

void Resizer::setRatioMode(bool ratioMode)
{
    ui->groupRatio->setChecked(ratioMode);
    ui->groupSize->setChecked(!ratioMode);
}

void Resizer::setSizeMode(bool sizeMode)
{
    ui->groupSize->setChecked(sizeMode);
    ui->groupRatio->setChecked(!sizeMode);
}

void Resizer::pressOpenFolder()
{
    QString path = QFileDialog::getExistingDirectory(this,tr("Select a Folder"),QDir::homePath());
    if(path.isEmpty()) return;
    QDir dir(path);
    QStringList filenames = dir.entryList(QStringList() << "*.jpg" << "*.JPG"<< "*.jpeg" << "*.JPEG" << "*.png" << "*.PNG");

    QStringList absoluteFilepaths;
    for(int i=0;i<filenames.size();i++)
        absoluteFilepaths << dir.absolutePath() + QDir::separator() + filenames.at(i);
    addList(absoluteFilepaths);
}

void Resizer::pressOpenFiles()
{
    QStringList paths = QFileDialog::getOpenFileNames(this,tr("Select Files"),QDir::homePath(), tr("Image files (*.jpg *.jpeg *.png)"));
    if(paths.isEmpty()) return;
    QStringList absoluteFilepaths;
    for(int i=0;i<paths.size();i++){
        QFileInfo fi(paths[i]);
        absoluteFilepaths << fi.absoluteFilePath();
    }
    addList(absoluteFilepaths);
}

void Resizer::addList(QStringList paths)
{
    if(paths.size()>1)
        diag_->show();

    for(int i=0;i<paths.size();i++){

        QString filepath = paths[i];
        QFileInfo fi(filepath);

        if(fi.isDir()){
            QDir dir(filepath);
            QStringList filenames = dir.entryList(QStringList() << "*.jpg" << "*.JPG"<< "*.jpeg" << "*.JPEG" << "*.png" << "*.PNG");

            QStringList absoluteFilepaths;
            for(int i=0;i<filenames.size();i++)
                absoluteFilepaths << dir.absoluteFilePath(filenames.at(i));
            addList(absoluteFilepaths);

            continue;
        }

        diag_->setMaximum( diag_->maximum() +1 );

        MyLabel *label = new MyLabel(fi.absoluteFilePath());
        label->setContextMenuPolicy(Qt::CustomContextMenu);
        connect(label,SIGNAL(customContextMenuRequested(QPoint)),this,SLOT(displayLabelMenu(QPoint)));
        int k = mapImages.size();
        ui->gridLayout->addWidget(label,k/5,k%5,Qt::AlignHCenter);

        ImageInfo *imageinfo = new ImageInfo;
        imageinfo->fileinfo = fi;
        imageinfo->label = label;

        mapImages.insert(fi.absoluteFilePath(),imageinfo);


        Loader * loader = new Loader;
        loader->setFileInfo(fi);

        connect(loader,SIGNAL(imageLoaded(QString,QImage)),this,SLOT(imageLoaded(QString,QImage)));

        QThreadPool::globalInstance()->start(loader);

    }

    if(diag_->maximum()==0)
        diag_->close();
}

void Resizer::addFile(QString filepath)
{
    addList(QStringList() << filepath);
}

void Resizer::removeFile(QString filepath)
{
    QFileInfo fi(filepath);

    mapImages.remove(fi.absoluteFilePath());

    repaintGrid();
}

void Resizer::imageLoaded(QString absoluteFilePath, QImage img)
{
    mapImages[absoluteFilePath]->preview = QPixmap::fromImage(img);

    QLabel *label = mapImages[absoluteFilePath]->label;
    label->setPixmap(mapImages[absoluteFilePath]->preview);
    label->setStatusTip(mapImages[absoluteFilePath]->fileinfo.fileName());
    label->setToolTip(mapImages[absoluteFilePath]->fileinfo.fileName());

    if(diag_->value()<0)
        diag_->setValue(1);
    else
        diag_->setValue( diag_->value() +1 );

    if(diag_->value()<0){
        diag_->close();
        diag_->setMaximum(0);
    }
}

void Resizer::resizeFinished(QString absoluteFilePath)
{
    mapImages.remove(absoluteFilePath);

    if(diag_->value()<0)
        diag_->setValue(1);
    else
        diag_->setValue( diag_->value() +1 );

    if(diag_->value()<0){
        diag_->close();
        diag_->setMaximum(0);
        this->close();
    }
}

void Resizer::displayLabelMenu(QPoint pt)
{
    QMenu *menu = new QMenu(this);
    QAction *action = menu->addAction(tr("Remove"),this,SLOT(removeImage()));
    action->setData(qobject_cast<MyLabel*>(sender())->getAbsoluteFilePath());

    menu->move( qobject_cast<QWidget*>(sender())->mapToGlobal(pt) );
    menu->show();
}

void Resizer::removeImage()
{
    QString absoluteFilePath = qobject_cast<QAction*>(sender())->data().toString();

    int index = ui->gridLayout->indexOf(mapImages[absoluteFilePath]->label);

    int row,col,rowSpan,colSpan;
    ui->gridLayout->getItemPosition(index,&row,&col,&rowSpan,&colSpan);

    for(int k = row*5+col+1; k<ui->gridLayout->count();k++){
        int prev = k-1;
        ui->gridLayout->addWidget(ui->gridLayout->itemAtPosition(k/5,k%5)->widget(),prev/5,prev%5);
    }


    mapImages[absoluteFilePath]->label->close();
    ui->gridLayout->removeWidget(mapImages[absoluteFilePath]->label);

    mapImages.remove(absoluteFilePath);
}

void Resizer::repaintGrid()
{
    int k =0;
    foreach(ImageInfo *img, mapImages){

        QLabel *label = new QLabel;
        label->setPixmap( img->preview );

        ui->gridLayout->addWidget(label,k/5,k%5,Qt::AlignHCenter);

        k++;
    }
}

int Resizer::readOrientation(QString filepath)
{
    if(QFile::exists(filepath)){
        return QExifImageHeader(filepath).value(QExifImageHeader::Orientation).toShort();
    }
    return 0;
}

void Resizer::openLogo()
{
    QString logo = QFileDialog::getOpenFileName(this,tr("Select Logo"),"",tr("Image files (*.jpg *.jpeg *.png)"));

    if(logo.isEmpty())
        return;

    setLogo(logo);
}

void Resizer::setLogo(QString path)
{
    logoPath = path;
    ui->editLogo->setText(path);

    if(logoPath.isEmpty() || !QFile::exists(logoPath)){
        ui->labelLogo->setText(tr("Logo"));
    }else{
        QPixmap pix(logoPath);
        ui->labelLogo->setPixmap(pix.scaled(400,200,Qt::KeepAspectRatio));
    }
}

void Resizer::resizeAll()
{
    if(!mapImages.isEmpty())
        diag_->show();

    foreach(ImageInfo *img, mapImages){
        diag_->setMaximum( diag_->maximum() +1 );

        Saver *saver = new Saver;
        saver->setFileInfo(img->fileinfo);
        saver->setNoResize(ui->checkNotResize->isChecked());
        saver->setNeedRotation(ui->checkRotate->isChecked());

        saver->setUseRatio(ui->groupRatio->isChecked());
        saver->setRatio( ui->comboRatio->currentText().toDouble() / 100.0 );
        saver->setSizeMax( ui->comboPixels->currentText().toInt() );

        saver->setAddLogo(ui->groupLogo->isChecked());
        if(ui->groupLogo->isChecked()){
            bool ok_X,ok_Y;

            int posX = ui->horizontalLineEdit->text().toInt(&ok_X);
            int posY = ui->verticalLineEdit->text().toInt(&ok_Y);

            QImage logo(logoPath);

            saver->setLogo(logo);
            saver->setLogoPosition(ui->selector->position(),posX,posY);
        }

        connect(saver,SIGNAL(resizeFinished(QString)),this,SLOT(resizeFinished(QString)));

        QThreadPool::globalInstance()->start(saver);
    }
}

void Resizer::pressAbout()
{
    QMessageBox *mess = new QMessageBox(this);
    mess->setWindowTitle(tr("About"));
    mess->setText(tr("Written by %1 (%2)\nVersion: %3","author, year, version").arg(QString::fromUtf8("Léo Baudouin"),"2013",version_));
    mess->setIcon(QMessageBox::Information);
    mess->exec();
}

void Resizer::restart(QString path)
{
    QProcess process;
    process.startDetached("\""+path+"\"");
    this->close();
}

void Resizer::handleMessage(const QString& message)
{
    enum Action { Nothing, Open, Print } action;

    action = Nothing;
    QString filename = message;
    if (message.toLower().startsWith("/print ")) {
        filename = filename.mid(7);
        action = Print;
    } else if (!message.isEmpty()) {
        action = Open;
    }
    if (action == Nothing) {
        emit needToShow();
        return;
    }

    switch(action) {
    case Print:
        QMessageBox::information(this,tr("Information"),filename);
    break;

    case Open:
    {
        addList(filename.split("\n"));
        emit needToShow();
    }
    break;
    default:
    break;
    };
}
