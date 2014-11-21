#include "resizer.h"
#include "ui_resizer.h"

Resizer::Resizer(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::Resizer), m_loadingMovie(":images/loading")
{
    m_loadingMovie.start();
    ui->setupUi(this);

    setToolButtonsEnabled(false);

    QCoreApplication::setAttribute(Qt::AA_DontShowIconsInMenus,false);
    this->setAttribute(Qt::WA_AlwaysShowToolTips,true);

    qRegisterMetaType<ImageData>("ImageData");

#ifdef Q_OS_WIN
    QDir dir("plugins/");
#else
    QDir dir("/usr/lib/resizer/");
#endif

    QStringList plugins;
    foreach(QString plugin, dir.entryList(QDir::Files | QDir::NoDotAndDotDot))
        plugins << dir.absoluteFilePath(plugin);

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
            }else{
                qDebug() << "Failed to load:" << plugin;
            }
        }
    }

    //Drop images
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
    listRatio << "10" << "20" << "30" << "33" << "40" << "50" << "60" << "66" << "70" << "80" << "90" << "125" << "150" << "200";
    ui->comboRatio->addItems( listRatio );
    ui->comboRatio->setCurrentIndex(3);

    QIntValidator *validatorNotNull = new QIntValidator(this);
    validatorNotNull->setRange(8,32768);
    ui->comboPixels->lineEdit()->setValidator(validatorNotNull);

    QIntValidator *validator = new QIntValidator(this);
    validator->setRange(0,32768);
    ui->horizontalLineEdit->setValidator(validator);
    ui->verticalLineEdit->setValidator(validator);

    //ToolButtons
    connect(ui->selectAllToolButton,SIGNAL(clicked()),this,SLOT(selectAll()));
    connect(ui->deselectAllToolButton,SIGNAL(clicked()),this,SLOT(deselectAll()));

    connect(ui->autoRotationToolButton,SIGNAL(clicked()),this,SLOT(detectRotation()));
    connect(ui->resetRotationToolButton,SIGNAL(clicked()),this,SLOT(resetRotation()));
    connect(ui->leftRotationToolButton,SIGNAL(clicked()),this,SLOT(rotateLeft()));
    connect(ui->rightRotationToolButton,SIGNAL(clicked()),this,SLOT(rotateRight()));
    connect(ui->removeToolButton,SIGNAL(clicked()),this,SLOT(removeImage()));
    connect(ui->deleteToolButton,SIGNAL(clicked()),this,SLOT(deleteImage()));

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

    diag_->setMaximum(0);
    addFile(path);
    //emit this->addFiles(QStringList() << path);
}

void Resizer::pressOpenFiles()
{
    QList<QByteArray> supported = QImageWriter::supportedImageFormats();
    //qDebug() << QImageReader::supportedImageFormats();
    //qDebug() << QImageWriter::supportedImageFormats();

    QStringList filters;
    foreach(QByteArray filter, supported){
        filters << "*." + QString(filter);
    }
    //qDebug() << filters.join(" ");

    QStringList paths = QFileDialog::getOpenFileNames(this, tr("Select Files"), QDir::homePath(), tr("Image files (%1)").arg(filters.join(" ")));
    if(paths.isEmpty()) return;
    QStringList absoluteFilepaths;
    for(int i=0;i<paths.size();i++){
        QFileInfo fi(paths[i]);
        absoluteFilepaths << fi.absoluteFilePath();
    }
    QCoreApplication::processEvents();

    //emit this->addFiles(absoluteFilepaths);
    diag_->setMaximum(0);
    addList(absoluteFilepaths);
}

void Resizer::addList(QStringList paths)
{
    if(paths.size()>1){
        diag_->setLabelText(tr("Loading..."));
        diag_->show();
    }

    setUpdatesEnabled(false);

    QThreadPool::globalInstance()->setMaxThreadCount( ui->numberOfThreadsSpinBox->value() );

    for(int i=0;i<paths.size();i++){

        QString filepath = paths[i];
        QFileInfo fi(filepath);

        //Parse directory
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

        //Avoid duplicates
        if(mapImages.contains(fi.absoluteFilePath()))
            continue;

        //Check image format
        QImageReader reader(fi.absoluteFilePath());
        if(!QImageWriter::supportedImageFormats().contains(reader.format())){
            continue;
        }

        //Create label
        ImageLabel *label = new ImageLabel(fi.absoluteFilePath());
        //label->setPixmap( m_loading );
        label->setMovie(&m_loadingMovie);
        label->setSizePolicy(QSizePolicy::Fixed,QSizePolicy::Fixed);
        label->setContextMenuPolicy(Qt::CustomContextMenu);
        connect(label,SIGNAL(customContextMenuRequested(QPoint)),this,SLOT(displayLabelMenu(QPoint)));
        connect(label,SIGNAL(removePressed(QString)),this,SLOT(removeImage(QString)));
        connect(label,SIGNAL(deletePressed(QString)),this,SLOT(deleteImage(QString)));
        connect(label,SIGNAL(selectionChanged(bool)),this,SLOT(selectionChanged()));
        int k = mapImages.size();
        ui->gridLayout->addWidget(label,k/nbColumns_,k%nbColumns_,Qt::AlignHCenter);

        //Add to list
        mapImages.insert(fi.absoluteFilePath(),label);

        //Update number of images
        diag_->setMaximum( diag_->maximum() + 1 );

        //Create loader
        Loader * loader = new Loader;
        loader->setFileInfo(fi);
        loader->setNeedRotation(ui->autoDetectRotationCheckBox->isChecked());

        connect(loader,SIGNAL(imageLoaded(QString,ImageData)),this,SLOT(imageLoaded(QString,ImageData)),Qt::QueuedConnection);

        QThreadPool::globalInstance()->start(loader);
    }

    setUpdatesEnabled(true);

    if(diag_->maximum()==0){
        diag_->close();
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

    mapImages[absoluteFilePath]->setPixmapAndRotation(QPixmap::fromImage(imgData.image), imgData.rotation );

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

void Resizer::selectionChanged()
{
    int nbSelected = 0;
    foreach(ImageLabel* label, mapImages){
        if(label->selected()){
            nbSelected++;
        }
    }

    setToolButtonsEnabled(nbSelected>0);
}

void Resizer::selectAll()
{
    foreach(ImageLabel* label, mapImages)
        label->setChecked();
}

void Resizer::deselectAll()
{
    foreach(ImageLabel* label, mapImages)
        label->setUnChecked();
}

QStringList Resizer::selected()
{
    QStringList absoluteFilePathList;
    foreach(ImageLabel* label, mapImages){
        if(label->isChecked()){
            absoluteFilePathList << label->getAbsoluteFilePath();
        }
    }
    return absoluteFilePathList;
}

void Resizer::displayLabelMenu(QPoint pt)
{
    //QString absoluteFilePath = qobject_cast<MyLabel*>(sender())->getAbsoluteFilePath();

    QStringList absoluteFilePathList = selected();

    QMenu *menu = new QMenu(this);
    menu->addAction(QIcon(":images/auto"),tr("Detect rotation"),this,SLOT(detectRotation()));
    menu->addAction(QIcon(":images/reset"),tr("Reset rotation"),this,SLOT(resetRotation()));
    menu->addAction(QIcon(":images/left"),tr("Rotate to left"),this,SLOT(rotateLeft()));
    menu->addAction(QIcon(":images/right"),tr("Rotate to right"),this,SLOT(rotateRight()));
    menu->addSeparator();
    menu->addAction(QIcon(":images/remove"),tr("Remove from grid"),this,SLOT(removeImage()));
    menu->addAction(QIcon(":images/delete"),tr("Delete file"),this,SLOT(deleteImage()));

    qDebug() << absoluteFilePathList;

    menu->move( qobject_cast<QWidget*>(sender())->mapToGlobal(pt) );
    if(menu->exec()){
        /*foreach(ImageLabel* label, mapImages){
            if(label->isChecked()){
                label->setUnChecked();
            }
        }*/
    }
}

void Resizer::removeImage()
{
    removeImage(selected());
}

void Resizer::removeImage(QString filename)
{
    removeImage(QStringList() << filename);
}

void Resizer::removeImage(QStringList absoluteFilePathList)
{
    foreach(QString absoluteFilePath, absoluteFilePathList){
        int index = ui->gridLayout->indexOf(mapImages[absoluteFilePath]);

        int row,col,rowSpan,colSpan;
        ui->gridLayout->getItemPosition(index,&row,&col,&rowSpan,&colSpan);

        for(int k = row*nbColumns_+col+1; k<ui->gridLayout->count();k++){
            int prev = k-1;
            ui->gridLayout->addWidget(ui->gridLayout->itemAtPosition(k/nbColumns_,k%nbColumns_)->widget(),prev/nbColumns_,prev%nbColumns_);
        }


        mapImages[absoluteFilePath]->close();
        ui->gridLayout->removeWidget(mapImages[absoluteFilePath]);

        mapImages.remove(absoluteFilePath);
        emit this->updateNumber( mapImages.count() );
    }

}

void Resizer::deleteImage()
{
    deleteImage(selected());
}

void Resizer::deleteImage(QString filename)
{
    deleteImage(QStringList() << filename);
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
    detectRotation(selected());
}

void Resizer::detectRotation(QStringList absoluteFilePathList)
{
    foreach(QString absoluteFilePath, absoluteFilePathList){
        mapImages[absoluteFilePath]->detectRotation();
    }
}

void Resizer::resetRotation()
{
    resetRotation(selected());
}

void Resizer::resetRotation(QStringList absoluteFilePathList)
{
    foreach(QString absoluteFilePath, absoluteFilePathList){
        mapImages[absoluteFilePath]->resetRotation();
    }
}

void Resizer::rotateLeft()
{
    rotateLeft(selected());
}

void Resizer::rotateLeft(QStringList absoluteFilePathList)
{
    foreach(QString absoluteFilePath, absoluteFilePathList){
        mapImages[absoluteFilePath]->rotateLeft();
    }
}

void Resizer::rotateRight()
{
    rotateRight(selected());
}

void Resizer::rotateRight(QStringList absoluteFilePathList)
{
    foreach(QString absoluteFilePath, absoluteFilePathList){
        mapImages[absoluteFilePath]->rotateRight();
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

    diag_->setMaximum( mapImages.size() );
    foreach(ImageLabel *label, mapImages){

        Saver *saver = new Saver;
        saver->setFileInfo(QFileInfo(label->getAbsoluteFilePath()));
        saver->setNoResize(ui->noResizeCheckBox->isChecked());
        saver->setRotation(label->rotation());
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

void Resizer::setToolButtonsEnabled(bool enabled)
{
    ui->autoRotationToolButton->setEnabled(enabled);
    ui->resetRotationToolButton->setEnabled(enabled);
    ui->leftRotationToolButton->setEnabled(enabled);
    ui->rightRotationToolButton->setEnabled(enabled);
    ui->removeToolButton->setEnabled(enabled);
    ui->deleteToolButton->setEnabled(enabled);
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
