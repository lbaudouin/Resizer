#include "resizer.h"
#include "ui_resizer.h"

Resizer::Resizer(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::Resizer)
{
    ui->setupUi(this);

    QCoreApplication::setAttribute(Qt::AA_DontShowIconsInMenus,false);
    this->setAttribute(Qt::WA_AlwaysShowToolTips,true);

    qRegisterMetaType<ImageData>("ImageData");

    QStringList plugins;
    plugins << QString("/usr/lib/resizer/libunity-plugin.so");
    //plugins << QString("plugins/libunity-plugin.so");

    foreach(QString plugin, plugins){
        if(QFile::exists(plugin)){
            QPluginLoader * loader = new QPluginLoader( plugin , this );
            if(loader->load()){
                Interface *interface = qobject_cast< Interface* >( loader->instance() ) ;
                if(interface){
                    connect(this,SIGNAL(finished()),interface,SLOT(finished()));
                    connect(this,SIGNAL(updateProgressBar(int,int,int)),interface,SLOT(updateProgressBar(int,int,int)));
                    connect(this,SIGNAL(updateNumber(int)),interface,SLOT(updateNumber(int)));

                    qDebug() << "Plugin connected:" << plugin;
                }
            }
        }
    }

    this->setAcceptDrops( true );

    nbColumns_ = this->width() / 350;

    diag_ = new QProgressDialog(QString(),tr("Cancel"),0,0,this);
    diag_->setWindowTitle(tr("Please wait"));
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

    ui->buttonOpenFiles->setIcon(QIcon(":images/pictures"));
    ui->buttonOpenFolder->setIcon(QIcon(":images/folder"));

    ui->actionAdd_files->setIcon(QIcon(":images/pictures"));
    ui->actionAdd_folder->setIcon(QIcon(":images/folder"));

    ui->buttonBox->addButton(tr("Resize"),QDialogButtonBox::AcceptRole);
    connect(ui->buttonBox,SIGNAL(rejected()),this,SLOT(close()));
    connect(ui->buttonBox,SIGNAL(accepted()),this,SLOT(resizeAll()));

    ui->numberOfThreadsSpinBox->setMaximum( QThread::idealThreadCount() );

    readSettings();

    setLogo(logoPath);

    connect(ui->actionAbout,SIGNAL(triggered()),this,SLOT(pressAbout()));

    connect(this,SIGNAL(addFiles(QStringList)),this,SLOT(addList(QStringList)),Qt::QueuedConnection);
}

Resizer::~Resizer()
{
    writeSettings();
    delete ui;
}

void Resizer::resizeEvent(QResizeEvent *)
{
    int prev = nbColumns_;
    nbColumns_ = this->width() / 350;
    if(prev!=nbColumns_){
        repaintGrid();
    }
}

void Resizer::writeSettings()
{
    QSettings settings(qAppName(), "config");

    settings.setValue("options/output_subfolder",ui->outputSubfolderLineEdit->text());
    settings.setValue("options/keep_exif",ui->keepExifInfoCheckBox->isChecked());
    settings.setValue("options/no_resize",ui->noResizeCheckBox->isChecked());
    settings.setValue("options/rotate",ui->autoDetectRotationCheckBox->isChecked());
    settings.setValue("options/auto_add_logo",ui->addLogoAutomaticalyCheckBox->isChecked());
    settings.setValue("options/nb_thread",ui->numberOfThreadsSpinBox->value());

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

    ui->outputSubfolderLineEdit->setText( settings.value("options/output_subfolder",tr("Small")).toString() );
    ui->keepExifInfoCheckBox->setChecked( settings.value("options/keep_exif",false).toBool() );
    ui->noResizeCheckBox->setChecked( settings.value("options/no_resize",false).toBool() );
    ui->autoDetectRotationCheckBox->setChecked( settings.value("options/rotate",true).toBool() );
    ui->addLogoAutomaticalyCheckBox->setChecked( settings.value("options/auto_add_logo",false).toBool() );
    if(settings.value("options/auto_add_logo",false).toBool())
        ui->groupLogo->setChecked(true);

    ui->numberOfThreadsSpinBox->setValue( settings.value("options/nb_thread",QThread::idealThreadCount()).toInt() );
    QThreadPool::globalInstance()->setMaxThreadCount( settings.value("options/nb_thread",QThread::idealThreadCount()).toInt() );


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

    QCoreApplication::processEvents();
    emit this->addFiles(QStringList() << path);
}

void Resizer::pressOpenFiles()
{
    QList<QByteArray> supported = QImageWriter::supportedImageFormats();

    QStringList filters;
    foreach(QByteArray filter, supported){
        filters << "*." + QString(filter);
    }
    qDebug() << filters.join(" ");

    QStringList paths = QFileDialog::getOpenFileNames(this, tr("Select Files"), QDir::homePath(), tr("Image files (%1)").arg(filters.join(" ")));
    if(paths.isEmpty()) return;
    QStringList absoluteFilepaths;
    for(int i=0;i<paths.size();i++){
        QFileInfo fi(paths[i]);
        absoluteFilepaths << fi.absoluteFilePath();
    }
    QCoreApplication::processEvents();
    emit this->addFiles(absoluteFilepaths);
}

void Resizer::addList(QStringList paths)
{
    if(paths.size()>1){
        diag_->setLabelText(tr("Loading..."));
        diag_->show();
    }

    ui->scrollAreaWidgetContents->hide();

    QThreadPool::globalInstance()->setMaxThreadCount( ui->numberOfThreadsSpinBox->value() );

    for(int i=0;i<paths.size();i++){

        QString filepath = paths[i];
        QFileInfo fi(filepath);

        if(fi.isDir()){
            QDir dir(filepath);
            if(!dir.exists())
                continue;

            dir.setFilter(QDir::Files);
            QStringList filenames = dir.entryList();

            QStringList absoluteFilepaths;
            for(int i=0;i<filenames.size();i++)
                absoluteFilepaths << dir.absoluteFilePath(filenames.at(i));
            addList(absoluteFilepaths);
            continue;
        }

        if(mapImages.contains(fi.absoluteFilePath()))
            continue;


        QImageReader reader(fi.absoluteFilePath());
        if(!QImageWriter::supportedImageFormats().contains(reader.format())){
            continue;
        }

        diag_->setMaximum( diag_->maximum() +1 );

        MyLabel *label = new MyLabel(fi.absoluteFilePath());
        label->setSizePolicy(QSizePolicy::Fixed,QSizePolicy::Fixed);
        label->setContextMenuPolicy(Qt::CustomContextMenu);
        connect(label,SIGNAL(customContextMenuRequested(QPoint)),this,SLOT(displayLabelMenu(QPoint)));
        connect(label,SIGNAL(autoPressed(QString)),this,SLOT(detectRotation(QString)));
        connect(label,SIGNAL(resetPressed(QString)),this,SLOT(resetRotation(QString)));
        connect(label,SIGNAL(leftPressed(QString)),this,SLOT(rotateLeft(QString)));
        connect(label,SIGNAL(rightPressed(QString)),this,SLOT(rotateRight(QString)));
        connect(label,SIGNAL(removePressed(QString)),this,SLOT(removeImage(QString)));
        connect(label,SIGNAL(deletePressed(QString)),this,SLOT(deleteImage(QString)));
        int k = mapImages.size();
        ui->gridLayout->addWidget(label,k/nbColumns_,k%nbColumns_,Qt::AlignHCenter);

        ImageInfo *imageinfo = new ImageInfo;
        imageinfo->fileinfo = fi;
        imageinfo->label = label;

        mapImages.insert(fi.absoluteFilePath(),imageinfo);


        Loader * loader = new Loader;
        loader->setFileInfo(fi);
        loader->setNeedRotation(ui->autoDetectRotationCheckBox->isChecked());

        connect(loader,SIGNAL(imageLoaded(QString,ImageData)),this,SLOT(imageLoaded(QString,ImageData)),Qt::QueuedConnection);

        QThreadPool::globalInstance()->start(loader);

    }

    if(diag_->maximum()==0){
        diag_->close();
        ui->scrollAreaWidgetContents->show();
    }
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

void Resizer::imageLoaded(QString absoluteFilePath, ImageData imgData)
{
    if(imgData.image.isNull()){
        qDebug() << "Failed to load:" << absoluteFilePath;
    }

    QImage img;
    if(imgData.rotation==NO_ROTATION){
        img = imgData.image;
    }else{
        QTransform transform;
        switch(imgData.rotation){
        case CLOCKWISE: transform.rotate(90); break;
        case REVERSE: transform.rotate(180); break;
        case COUNTERCLOCKWISE: transform.rotate(270); break;
        default: break;
        }
        img = imgData.image.transformed(transform);
    }

    mapImages[absoluteFilePath]->rotation = imgData.rotation;

    QLabel *label = mapImages[absoluteFilePath]->label;
    label->setPixmap(QPixmap::fromImage(img));

    emit this->updateNumber( mapImages.count() );

    if(diag_->value()<0)
        diag_->setValue(1);
    else
        diag_->setValue( diag_->value() +1 );

    emit this->updateProgressBar(diag_->minimum(),diag_->maximum(),diag_->value());

    if(diag_->value()<0){
        diag_->close();
        diag_->setMaximum(0);
        ui->scrollAreaWidgetContents->show();
        ui->scrollAreaWidgetContents->layout()->update();
    }
}

void Resizer::resizeFinished(QString absoluteFilePath)
{
    mapImages.remove(absoluteFilePath);

    if(diag_->value()<0)
        diag_->setValue(1);
    else
        diag_->setValue( diag_->value() +1 );

    emit this->updateProgressBar(diag_->minimum(),diag_->maximum(),diag_->value());

    if(diag_->value()<0){
        diag_->close();
        diag_->setMaximum(0);
        this->hide();
        emit this->finished();
        this->close();
    }
}

void Resizer::displayLabelMenu(QPoint pt)
{
    //QString absoluteFilePath = qobject_cast<MyLabel*>(sender())->getAbsoluteFilePath();

    QStringList absoluteFilePathList;
    foreach(ImageInfo* info, mapImages){
        if(info->label->isChecked()){
            absoluteFilePathList << info->fileinfo.absoluteFilePath();
            info->label->setUnChecked();
        }
    }


    QMenu *menu = new QMenu(this);
    QAction *actionAuto = menu->addAction(QIcon(":images/auto"),tr("Detect rotation"),this,SLOT(detectRotation()));
    QAction *actionReset = menu->addAction(QIcon(":images/reset"),tr("Reset rotation"),this,SLOT(resetRotation()));
    QAction *actionLeft = menu->addAction(QIcon(":images/left"),tr("Rotate to left"),this,SLOT(rotateLeft()));
    QAction *actionRigth = menu->addAction(QIcon(":images/right"),tr("Rotate to right"),this,SLOT(rotateRight()));
    menu->addSeparator();
    QAction *actionRemove = menu->addAction(QIcon(":images/remove"),tr("Remove from grid"),this,SLOT(removeImage()));
    QAction *actionDelete = menu->addAction(QIcon(":images/delete"),tr("Delete file"),this,SLOT(deleteImage()));


    actionAuto->setData(absoluteFilePathList);
    actionReset->setData(absoluteFilePathList);
    actionLeft->setData(absoluteFilePathList);
    actionRigth->setData(absoluteFilePathList);
    actionRemove->setData(absoluteFilePathList);
    actionDelete->setData(absoluteFilePathList);

    menu->move( qobject_cast<QWidget*>(sender())->mapToGlobal(pt) );
    menu->show();
}

void Resizer::removeImage()
{
    QAction *action = qobject_cast<QAction*>(sender());
    if(action){
        QStringList absoluteFilePathList = qobject_cast<QAction*>(sender())->data().toStringList();
        removeImage(absoluteFilePathList);
    }
}

void Resizer::removeImage(QString absoluteFilePath)
{
    removeImage(QStringList() << absoluteFilePath);
}

void Resizer::removeImage(QStringList absoluteFilePathList)
{
    foreach(QString absoluteFilePath, absoluteFilePathList){
        int index = ui->gridLayout->indexOf(mapImages[absoluteFilePath]->label);

        int row,col,rowSpan,colSpan;
        ui->gridLayout->getItemPosition(index,&row,&col,&rowSpan,&colSpan);

        for(int k = row*nbColumns_+col+1; k<ui->gridLayout->count();k++){
            int prev = k-1;
            ui->gridLayout->addWidget(ui->gridLayout->itemAtPosition(k/nbColumns_,k%nbColumns_)->widget(),prev/nbColumns_,prev%nbColumns_);
        }


        mapImages[absoluteFilePath]->label->close();
        ui->gridLayout->removeWidget(mapImages[absoluteFilePath]->label);

        mapImages.remove(absoluteFilePath);
        emit this->updateNumber( mapImages.count() );
    }

}

void Resizer::deleteImage()
{
    QAction *action = qobject_cast<QAction*>(sender());
    if(action){
        QStringList absoluteFilePathList = qobject_cast<QAction*>(sender())->data().toStringList();
        deleteImage(absoluteFilePathList);
    }
}

void Resizer::deleteImage(QString absoluteFilePath)
{
    deleteImage(QStringList() << absoluteFilePath);
}

void Resizer::deleteImage(QStringList absoluteFilePathList)
{
    if(absoluteFilePathList.isEmpty())
        return;

    QMessageBox *mess = new QMessageBox(this);
    mess->setIcon(QMessageBox::Warning);
    mess->setWindowTitle(tr("Delete file"));
    if(absoluteFilePathList.size()>1){
        mess->setText(tr("This will delete the image files from your computer.\nAre you sure to continue ?"));
        mess->setDetailedText(absoluteFilePathList.join("\n"));
    }else{
        mess->setText(tr("This will delete the image file '%1' from your computer.\nAre you sure to continue ?").arg(QFileInfo(absoluteFilePathList.first()).fileName()));
    }
    QPushButton *cancelButton = mess->addButton(QMessageBox::Cancel);
    QPushButton *trashButton = mess->addButton(tr("Move to Trash"),QMessageBox::YesRole);
    trashButton->setDisabled(true);
    QPushButton *deleteButton = mess->addButton(tr("Remove file"),QMessageBox::AcceptRole);

    if( mess->exec() ){
        if(mess->clickedButton() == cancelButton){
            return;
        }

        foreach(QString absoluteFilePath, absoluteFilePathList){
            if(mess->clickedButton() == trashButton){
                //TODO
            }
            if(mess->clickedButton() == deleteButton){
                QFile::remove(absoluteFilePath);
            }
            removeImage(absoluteFilePath);
        }
    }
}

void Resizer::detectRotation()
{
    QAction *action = qobject_cast<QAction*>(sender());
    if(action){
        QStringList absoluteFilePathList = qobject_cast<QAction*>(sender())->data().toStringList();
        detectRotation(absoluteFilePathList);
    }
}

void Resizer::detectRotation(QString absoluteFilePath)
{
    detectRotation(QStringList() << absoluteFilePath);
}

void Resizer::detectRotation(QStringList absoluteFilePathList)
{
    foreach(QString absoluteFilePath, absoluteFilePathList){
        int orientation = QExifImageHeader(absoluteFilePath).value(QExifImageHeader::Orientation).toShort();

        RotationState rotation;
        switch(orientation){
        case 6: rotation = CLOCKWISE; break;
        case 3: rotation = REVERSE; break;
        case 8: rotation = COUNTERCLOCKWISE; break;
        default: rotation = NO_ROTATION;
        }

        if(rotation==mapImages[absoluteFilePath]->rotation)
            continue;

        QTransform transform;
        switch(mapImages[absoluteFilePath]->rotation){
        case CLOCKWISE: transform.rotate(-90); break;
        case REVERSE: transform.rotate(-180); break;
        case COUNTERCLOCKWISE: transform.rotate(-270); break;
        default: break;
        }
        switch(rotation){
        case CLOCKWISE: transform.rotate(90); break;
        case REVERSE: transform.rotate(180); break;
        case COUNTERCLOCKWISE: transform.rotate(270); break;
        default: break;
        }

        QPixmap pix = mapImages[absoluteFilePath]->label->pixmap()->copy();
        pix = pix.transformed(transform);
        mapImages[absoluteFilePath]->rotation = rotation;

        mapImages[absoluteFilePath]->label->setPixmap(pix);
    }
}

void Resizer::resetRotation()
{
    QAction *action = qobject_cast<QAction*>(sender());
    if(action){
        QStringList absoluteFilePathList = qobject_cast<QAction*>(sender())->data().toStringList();
        resetRotation(absoluteFilePathList);
    }
}

void Resizer::resetRotation(QString absoluteFilePath)
{
    resetRotation(QStringList() << absoluteFilePath);
}

void Resizer::resetRotation(QStringList absoluteFilePathList)
{
    foreach(QString absoluteFilePath, absoluteFilePathList){
        if(mapImages[absoluteFilePath]->rotation==NO_ROTATION)
            continue;

        QTransform transform;
        switch(mapImages[absoluteFilePath]->rotation){
        case CLOCKWISE: transform.rotate(-90); break;
        case REVERSE: transform.rotate(-180); break;
        case COUNTERCLOCKWISE: transform.rotate(-270); break;
        default: break;
        }

        QPixmap pix = mapImages[absoluteFilePath]->label->pixmap()->copy();
        pix = pix.transformed(transform);

        mapImages[absoluteFilePath]->label->setPixmap(pix);
        mapImages[absoluteFilePath]->rotation = NO_ROTATION;
    }
}

void Resizer::rotateLeft()
{
    QAction *action = qobject_cast<QAction*>(sender());
    if(action){
        QStringList absoluteFilePathList = qobject_cast<QAction*>(sender())->data().toStringList();
        rotateLeft(absoluteFilePathList);
    }
}

void Resizer::rotateLeft(QString absoluteFilePath)
{
    rotateLeft(QStringList() << absoluteFilePath);
}

void Resizer::rotateLeft(QStringList absoluteFilePathList)
{
    foreach(QString absoluteFilePath, absoluteFilePathList){
        QTransform transform;
        transform.rotate(-90);

        QPixmap pix = mapImages[absoluteFilePath]->label->pixmap()->copy();
        pix = pix.transformed(transform);

        mapImages[absoluteFilePath]->label->setPixmap(pix);

        RotationState rotation;
        if(mapImages[absoluteFilePath]->rotation==NO_ROTATION){
            rotation = COUNTERCLOCKWISE;
        }else{
            rotation = static_cast<RotationState>(mapImages[absoluteFilePath]->rotation-1);
        }

        mapImages[absoluteFilePath]->rotation = rotation;
    }
}

void Resizer::rotateRight()
{
    QAction *action = qobject_cast<QAction*>(sender());
    if(action){
        QStringList absoluteFilePathList = qobject_cast<QAction*>(sender())->data().toStringList();
        rotateRight(absoluteFilePathList);
    }
}

void Resizer::rotateRight(QString absoluteFilePath)
{
    rotateRight(QStringList() << absoluteFilePath);
}

void Resizer::rotateRight(QStringList absoluteFilePathList)
{
    foreach(QString absoluteFilePath, absoluteFilePathList){
        QTransform transform;
        transform.rotate(90);

        QPixmap pix = mapImages[absoluteFilePath]->label->pixmap()->copy();
        pix = pix.transformed(transform);

        mapImages[absoluteFilePath]->label->setPixmap(pix);

        RotationState rotation = static_cast<RotationState>((mapImages[absoluteFilePath]->rotation+1)%4);

        mapImages[absoluteFilePath]->rotation = rotation;
    }
}

void Resizer::repaintGrid()
{
    QList<QLayoutItem*> list;
    for(int k=0;k<ui->gridLayout->count();k++){
        list << ui->gridLayout->itemAt(k);
    }

    for(int k=0;k<list.size();k++){
        ui->gridLayout->addWidget(list.at(k)->widget(),k/nbColumns_,k%nbColumns_,Qt::AlignHCenter);
    }
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
    if(ui->outputSubfolderLineEdit->text().isEmpty()){
        QMessageBox::warning(this,tr("Warning"),tr("You need to set an output subfolder name"));
        return;
    }

    if(!mapImages.isEmpty()){
        diag_->setLabelText(tr("Resizing..."));
        diag_->show();
    }

    QThreadPool::globalInstance()->setMaxThreadCount( ui->numberOfThreadsSpinBox->value() );

    foreach(ImageInfo *img, mapImages){
        diag_->setMaximum( diag_->maximum() +1 );

        Saver *saver = new Saver;
        saver->setFileInfo(img->fileinfo);
        saver->setNoResize(ui->noResizeCheckBox->isChecked());
        saver->setRotation(img->rotation);
        saver->setOutputSubfolder(ui->outputSubfolderLineEdit->text());

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

        connect(saver,SIGNAL(resizeFinished(QString)),this,SLOT(resizeFinished(QString)),Qt::QueuedConnection);

        QThreadPool::globalInstance()->start(saver);
    }
}

void Resizer::dropEvent(QDropEvent *event)
{
    QList<QUrl> filelist = event->mimeData()->urls() ;

    QStringList files;

    foreach(QUrl url, filelist){
        files << url.toLocalFile();
    }

    addList(files);
}

void Resizer::dragEnterEvent(QDragEnterEvent *event)
{
    event->acceptProposedAction();
}

void Resizer::dragMoveEvent(QDragMoveEvent* event)
{
    event->acceptProposedAction();
}

void Resizer::dragLeaveEvent(QDragLeaveEvent* event)
{
    event->accept();
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
