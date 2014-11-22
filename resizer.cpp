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

    //Initialize
    nbColumns_ = this->width() / 350;

    //Loading dialog
    diag_ = new QProgressDialog(QString(),tr("Cancel"),0,0,this);
    diag_->setWindowTitle(tr("Please wait"));
    diag_->setValue(0);
    diag_->setMinimumDuration(50);

    //Initialize ratio/max size combo
    QStringList listSize;
    listSize << "320" << "480" << "640" << "720" << "800" << "1024" << "1280" << "2048" << "4096";
    ui->comboPixels->addItems( listSize );
    ui->comboPixels->setCurrentIndex(5);
    QStringList listRatio;
    listRatio << "10" << "20" << "30" << "33" << "40" << "50" << "60" << "66" << "70" << "80" << "90" << "125" << "150" << "200";
    ui->comboRatio->addItems( listRatio );
    ui->comboRatio->setCurrentIndex(3);

    //Set validators
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

    //Buttons
    ui->buttonOpenFiles->setIcon(QIcon(":images/pictures"));
    ui->buttonOpenFolder->setIcon(QIcon(":images/folder"));

    //Actions
    ui->actionAdd_files->setIcon(QIcon(":images/pictures"));
    ui->actionAdd_folder->setIcon(QIcon(":images/folder"));

    ui->buttonBox->addButton(tr("Resize"),QDialogButtonBox::AcceptRole);
    connect(ui->buttonBox,SIGNAL(rejected()),this,SLOT(close()));
    connect(ui->buttonBox,SIGNAL(accepted()),this,SLOT(resizeAll()));

    ui->numberOfThreadsSpinBox->setMaximum( QThread::idealThreadCount() );

    readSettings();

    setLogo(m_logoPath);

    connect(ui->actionAbout,SIGNAL(triggered()),this,SLOT(pressAbout()));

    ui->status->clear();

    imageSaver = new QFutureWatcher<bool>(this);

    m_resizeProgressDialog = new QProgressDialog(tr("Resizing..."),tr("Cancel"),0,0,this);
    m_resizeProgressDialog->setWindowTitle(tr("Please wait"));

    connect(imageSaver,SIGNAL(progressRangeChanged(int,int)),m_resizeProgressDialog,SLOT(setRange(int,int)));
    connect(imageSaver,SIGNAL(progressValueChanged(int)),m_resizeProgressDialog,SLOT(setValue(int)));
    connect(m_resizeProgressDialog,SIGNAL(canceled()),imageSaver,SLOT(cancel()));
    connect(imageSaver,SIGNAL(finished()),this,SLOT(resizeFinished()));
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
    settings.setValue("options/close_after_resizing",ui->closeAfterResizingCheckBox->isChecked());

    settings.setValue("small/mode",ui->groupRatio->isChecked()?"ratio":"size");
    settings.setValue("small/ratio",ui->comboRatio->currentText());
    settings.setValue("small/pixels",ui->comboPixels->currentText());

    settings.setValue("logo/attach",ui->selector->position());
    settings.setValue("logo/shift-x",ui->horizontalLineEdit->text().toInt());
    settings.setValue("logo/shift-y",ui->verticalLineEdit->text().toInt());
    settings.setValue("logo/path",m_logoPath);
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

    ui->closeAfterResizingCheckBox->setChecked( settings.value("options/close_after_resizing",true).toBool() );

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
    m_logoPath = settings.value("logo/path",QDesktopServices::storageLocation(QDesktopServices::PicturesLocation)  + QDir::separator() + "logo.png").toString();
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

    ui->status->setText(tr("%n images","",mapImages.size()));
}

void Resizer::selectionChanged()
{
    int nbSelected = 0;
    foreach(ImageLabel* label, mapImages){
        if(label->selected()){
            nbSelected++;
        }
    }

    if(nbSelected>0)
        ui->status->setText(tr("%n images","",mapImages.size()) + " (" + tr("%n selected","",nbSelected) + ")");
    else
        ui->status->setText(tr("%n images","",mapImages.size()));

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
        ui->status->setText(tr("%n images","",mapImages.size()));
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
    m_logoPath = path;
    ui->editLogo->setText(path);

    if(m_logoPath.isEmpty() || !QFile::exists(m_logoPath)){
        ui->labelLogo->setText(tr("Logo"));
    }else{
        QPixmap pix(m_logoPath);
        ui->labelLogo->setPixmap(pix.scaled(400,200,Qt::KeepAspectRatio));
    }
}

void Resizer::resizeAll()
{
    if(ui->outputSubfolderLineEdit->text().isEmpty()){
        QMessageBox::warning(this,tr("Warning"),tr("You need to set an output subfolder name"));
        return;
    }

    if(mapImages.isEmpty())
        return;

    ImageInfo defaultInfo;
    defaultInfo.noResize = ui->noResizeCheckBox->isChecked();
    defaultInfo.outputFolder = ui->outputSubfolderLineEdit->text();
    defaultInfo.keepExif = ui->keepExifInfoCheckBox->isChecked();

    defaultInfo.useRatio = ui->groupRatio->isChecked();
    defaultInfo.ratio = ui->comboRatio->currentText().toDouble() / 100.0;
    defaultInfo.sizeMax = ui->comboPixels->currentText().toInt();

    defaultInfo.addLogo = ui->groupLogo->isChecked();
    if(defaultInfo.addLogo){
        bool ok_X,ok_Y;

        int posX = ui->horizontalLineEdit->text().toInt(&ok_X);
        int posY = ui->verticalLineEdit->text().toInt(&ok_Y);

        if(!ok_X || !ok_Y)
            defaultInfo.addLogo = false;

        QImage logo(m_logoPath);

        defaultInfo.logo = logo;
        defaultInfo.logoPosition = ui->selector->position();
        defaultInfo.logoShifting = QPoint(posX,posY);
    }

    QList<ImageInfo> images;
    foreach(ImageLabel *label, mapImages){
        ImageInfo info = defaultInfo;
        info.filename = label->getAbsoluteFilePath();
        info.rotation = label->rotation();
        images << info;

        //Create output folder here because mkdir is not thread-safe
        QDir dir = QFileInfo(info.filename).absoluteDir();
        dir.mkpath(info.outputFolder);
    }

    QThreadPool::globalInstance()->setMaxThreadCount(ui->numberOfThreadsSpinBox->value());

    imageSaver->setFuture(QtConcurrent::mapped(images, save));
    m_resizeProgressDialog->show();
}

void Resizer::resizeFinished()
{
    emit this->finished();
    if(ui->closeAfterResizingCheckBox->isChecked() && !m_resizeProgressDialog->wasCanceled()){
        this->close();
    }
}

bool Resizer::save(ImageInfo info)
{
    QFileInfo fi(info.filename);
    QString output = fi.absoluteDir().absolutePath() + QDir::separator() + info.outputFolder + QDir::separator() + fi.fileName();

    QDir dir(fi.absoluteDir());
    if(!dir.exists()){
        qDebug() << fi.absoluteDir().absolutePath() + " doesn't exist";
        return false;
    }

    QImage small;
    QImageReader reader(info.filename);
    QSize imageSize = reader.size();

    if(info.noResize){
        small.load(info.filename);
    }else{
        if(imageSize.isValid()){
            if(info.useRatio){
                imageSize *= info.ratio;
            }else{
                imageSize.scale(info.sizeMax,info.sizeMax,Qt::KeepAspectRatio);
            }
            reader.setScaledSize(imageSize);
            small = reader.read();
        }else{
            QImage original(info.filename);
            imageSize = original.size();
            if(info.useRatio){
                imageSize *= info.ratio;
            }else{
                imageSize.scale(info.sizeMax,info.sizeMax,Qt::KeepAspectRatio);
            }
            small = original.scaled(imageSize,Qt::KeepAspectRatio);
        }
    }

    if(info.rotation!=NO_ROTATION){
        QTransform transform;
        switch(info.rotation){
        case CLOCKWISE: transform.rotate(90); break;
        case REVERSE: transform.rotate(180); break;
        case COUNTERCLOCKWISE: transform.rotate(270); break;
        default: transform.reset();
        }
        small = small.transformed(transform);
    }

    QPoint shift;
    if(info.addLogo && !info.logo.isNull()){
        switch(info.logoPosition){
        case PositionSelector::TOP_LEFT:
            break;
        case PositionSelector::TOP_RIGHT:
            shift.setX( small.width() - info.logo.width() - info.logoShifting.x() );
            break;
        case PositionSelector::BOTTOM_LEFT:
            shift.setY( small.height() - info.logo.height() - info.logoShifting.y() );
            break;
        case PositionSelector::BOTTOM_RIGHT:
            shift.setX( small.width() - info.logo.width() - info.logoShifting.x() );
            shift.setY( small.height() - info.logo.height() - info.logoShifting.y() );
            break;
        case PositionSelector::CENTER:
            shift.setX( small.width()/2.0 - info.logo.width()/2.0 + info.logoShifting.x() );
            shift.setY( small.height()/2.0 - info.logo.height()/2.0 + info.logoShifting.y() );
            break;
        default: break;
        }

        QPainter painter(&small);
        painter.drawImage(shift,info.logo);
        painter.end();
    }

    small.save(output);

    if(info.keepExif){
        QExifImageHeader exif(info.filename);
        QList<QExifImageHeader::ImageTag> list1 = exif.imageTags();
        QList<QExifImageHeader::ExifExtendedTag> list2 = exif.extendedTags();
        QList<QExifImageHeader::GpsTag> list3 = exif.gpsTags();

        /*for(int i=0;i<list1.size();i++){
            qDebug() << exif.value(list1[i]).toString();
        }
        for(int i=0;i<list2.size();i++){
            qDebug() << exif.value(list2[i]).toString();
        }
        for(int i=0;i<list3.size();i++){
            qDebug() << exif.value(list3[i]).toString();
        }*/

        //exif.setValue(QExifImageHeader::Orientation,0);
        exif.setValue(QExifImageHeader::ImageWidth,small.width());
        exif.setValue(QExifImageHeader::ImageLength,small.height());
        exif.setValue(QExifImageHeader::PixelXDimension,small.width());
        exif.setValue(QExifImageHeader::PixelYDimension,small.height());
        exif.setThumbnail(QImage());

        //exif.saveToJpeg(output);
    }

    //qDebug() << "Save: " << output;
    return true;
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
    mess->setText(tr("Written by %1 (%2)\nVersion: %3","author, year, version").arg(QString::fromUtf8("Léo Baudouin"),"2014",m_version));
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
