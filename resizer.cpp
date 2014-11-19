#include "resizer.h"
#include "ui_resizer.h"

Q_DECLARE_METATYPE( QList<int> )

Resizer::Resizer(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::Resizer), m_idCount(0)
{
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

    //Set model
    m_model = new ImagePreviewModel(QSize(320,320));
    ui->listView->setModel(m_model);

    connect(ui->listView->selectionModel(),SIGNAL(selectionChanged(QItemSelection,QItemSelection)),this,SLOT(selectionChanged()));

    //Set delegate
    m_delegate = new ImagePreviewDelegate;
    ui->listView->setItemDelegate(m_delegate);

    //Drop images
    this->setAcceptDrops( true );

    //Context menu
    connect(ui->listView,SIGNAL(customContextMenuRequested(QPoint)),this,SLOT(displayLabelMenu(QPoint)));

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

#if 1
    if(QFile::exists("test.jpg")){
      addFile("test.jpg");
    }
#endif
}

Resizer::~Resizer()
{
    writeSettings();
    delete ui;
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
    addList(absoluteFilepaths);
}

void Resizer::addList(QStringList paths)
{
    if(paths.size()>1){
        diag_->setLabelText(tr("Loading..."));
        diag_->show();
    }

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
        if(m_model->contains(fi.absoluteFilePath()))
            continue;

        //Check image format
        QImageReader reader(fi.absoluteFilePath());
        if(!QImageWriter::supportedImageFormats().contains(reader.format())){
            continue;
        }

        //Update number of images
        diag_->setMaximum( diag_->maximum() +1 );

        //Add to list
        int id = m_idCount++;
        m_model->addImage(id,fi.absoluteFilePath());

        //Create loader
        Loader * loader = new Loader;
        loader->setImageID(id);
        loader->setFileInfo(fi);
        loader->setNeedRotation(ui->autoDetectRotationCheckBox->isChecked());

        connect(loader,SIGNAL(imageLoaded(int,ImageData)),this,SLOT(imageLoaded(int,ImageData)),Qt::QueuedConnection);

        QThreadPool::globalInstance()->start(loader);
    }

    if(diag_->maximum()==0){
        diag_->close();
    }
}

void Resizer::addFile(QString filepath)
{
    addList(QStringList() << filepath);
}

void Resizer::imageLoaded(int id, ImageData imgData)
{
    if(imgData.image.isNull()){
        qDebug() << "Failed to load:" << id;
        return;
    }

    m_model->setImage(id,imgData.image,imgData.rotation);

    emit this->updateNumber( m_model->rowCount() );

    if(diag_->value()<0)
        diag_->setValue(1);
    else
        diag_->setValue( diag_->value() +1 );

    emit this->updateProgressBar(diag_->minimum(),diag_->maximum(),diag_->value());

    if(diag_->value()<0){
        diag_->close();
        diag_->setMaximum(0);
    }
}

void Resizer::resizeFinished(int id)
{
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
    setToolButtonsEnabled(ui->listView->selectionModel()->selectedIndexes().size()>0);
}

void Resizer::displayLabelMenu(QPoint pt)
{
    if(ui->listView->selectionModel()->selectedIndexes().size()<=0)
        return;

    QMenu *menu = new QMenu(this);
    /*QAction *actionAuto =*/ menu->addAction(QIcon(":images/auto"),tr("Detect rotation"),this,SLOT(detectRotation()));
    /*QAction *actionReset =*/ menu->addAction(QIcon(":images/reset"),tr("Reset rotation"),this,SLOT(resetRotation()));
    /*QAction *actionLeft =*/ menu->addAction(QIcon(":images/left"),tr("Rotate to left"),this,SLOT(rotateLeft()));
    /*QAction *actionRigth =*/ menu->addAction(QIcon(":images/right"),tr("Rotate to right"),this,SLOT(rotateRight()));
    menu->addSeparator();
    /*QAction *actionRemove =*/ menu->addAction(QIcon(":images/remove"),tr("Remove from grid"),this,SLOT(removeImage()));
    /*QAction *actionDelete =*/ menu->addAction(QIcon(":images/delete"),tr("Delete file"),this,SLOT(deleteImage()));

    menu->move( qobject_cast<QWidget*>(sender())->mapToGlobal(pt) );

    QAction *selectedAction = menu->exec();

    if(selectedAction){
        qDebug() << selectedAction->text();
    }
}

QList<int> Resizer::selectedIDs()
{
    QModelIndexList list = ui->listView->selectionModel()->selectedIndexes();

    QList<int> ids;
    foreach(QModelIndex index, list){
        ids << index.data(Qt::UserRole).toInt();
    }

    return ids;
}

void Resizer::removeImage()
{
    removeImage(selectedIDs());
}

void Resizer::removeImage(QList<int> ids)
{
    foreach(int id, ids){
        m_model->removeImage(id);
    }

    //Update number of images
    emit this->updateNumber( m_model->rowCount() );
}

void Resizer::deleteImage()
{
    deleteImage(selectedIDs());
}

void Resizer::deleteImage(QList<int> ids)
{
    if(ids.isEmpty())
        return;

    QStringList absoluteFilePathList;
    foreach(int id, ids){
        absoluteFilePathList << m_model->data(id,Qt::UserRole+1).toString();
    }

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
            removeImage(QList<int>() << ids.at(absoluteFilePathList.indexOf(absoluteFilePath)) );
        }
    }
}

void Resizer::detectRotation()
{
    detectRotation(selectedIDs());
}


void Resizer::detectRotation(QList<int> ids)
{
    foreach(int id, ids){
        QString filepath = m_model->getImageFilepath(id);
        if(!filepath.isEmpty()){
            int orientation = QExifImageHeader(filepath).value(QExifImageHeader::Orientation).toShort();

            RotationState rotation;
            switch(orientation){
            case 6: rotation = CLOCKWISE; break;
            case 3: rotation = REVERSE; break;
            case 8: rotation = COUNTERCLOCKWISE; break;
            default: rotation = NO_ROTATION;
            }

            m_model->setRotation(id,rotation);
        }
    }
}

void Resizer::resetRotation()
{
    resetRotation(selectedIDs());
}

void Resizer::resetRotation(QList<int> ids)
{
    foreach(int id, ids){
        m_model->setRotation(id,NO_ROTATION);
    }
}

void Resizer::rotateLeft()
{
    rotateLeft(selectedIDs());
}

void Resizer::rotateLeft(QList<int> ids)
{
    foreach(int id, ids){
        m_model->rotate(id,ROTATION_LEFT);
    }
}

void Resizer::rotateRight()
{
    rotateRight(selectedIDs());
}

void Resizer::rotateRight(QList<int> ids)
{
    foreach(int id, ids){
        m_model->rotate(id,ROTATION_RIGHT);
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

    if(m_model->rowCount()<=0){
        return;
    }

    diag_->setLabelText(tr("Resizing..."));
    diag_->show();
    diag_->setMaximum( m_model->rowCount() );

    QThreadPool::globalInstance()->setMaxThreadCount( ui->numberOfThreadsSpinBox->value() );

    QList<ImageInfo> images = m_model->getImageList();
    foreach(ImageInfo image, images){

        Saver *saver = new Saver;
        saver->setFileInfo(image.fileinfo);
        saver->setRotation(image.rotation);
        saver->setNoResize(ui->noResizeCheckBox->isChecked());
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

        connect(saver,SIGNAL(resizeFinished(int)),this,SLOT(resizeFinished(int)),Qt::QueuedConnection);

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
