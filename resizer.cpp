#include "resizer.h"
#include "ui_resizer.h"

Resizer::Resizer(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::Resizer)
{
    ui->setupUi(this);

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
    ui->posXLineEdit->setValidator(validator);
    ui->posYLineEdit->setValidator(validator);



    connect(ui->buttonOpenFolder,SIGNAL(pressed()),this,SLOT(pressOpenFolder()));
    connect(ui->buttonOpenFiles,SIGNAL(pressed()),this,SLOT(pressOpenFiles()));
    connect(ui->actionAdd_folder,SIGNAL(triggered()),this,SLOT(pressOpenFolder()));
    connect(ui->actionAdd_files,SIGNAL(triggered()),this,SLOT(pressOpenFiles()));

    connect(ui->groupRatio,SIGNAL(clicked(bool)),this,SLOT(setRatioMode(bool)));
    connect(ui->groupSize,SIGNAL(clicked(bool)),this,SLOT(setSizeMode(bool)));

    connect(ui->toolButton,SIGNAL(clicked()),this,SLOT(openLogo()));

    /*QButtonGroup *buttonGroup = new QButtonGroup;
    buttonGroup->addButton(ui->radioTopLeft);
    buttonGroup->addButton(ui->radioBottomRight);
    buttonGroup->setExclusive(true);*/

    ui->buttonOpenFiles->setText("");
    ui->buttonOpenFolder->setText("");
    ui->buttonOpenFiles->setIcon(QIcon(":images/pictures"));
    ui->buttonOpenFolder->setIcon(QIcon(":images/folder"));


    ui->actionAdd_files->setIcon(QIcon(":images/pictures"));
    ui->actionAdd_folder->setIcon(QIcon(":images/folder"));

    connect(ui->buttonBox,SIGNAL(rejected()),this,SLOT(close()));
    connect(ui->buttonBox,SIGNAL(accepted()),this,SLOT(resizeAll()));



    if(QFile::exists(QDesktopServices::storageLocation(QDesktopServices::PicturesLocation) + QDir::separator() + "logo.png")){
        logoPath = QDesktopServices::storageLocation(QDesktopServices::PicturesLocation)  + QDir::separator() + "logo.png";
        QPixmap pix(logoPath);
        ui->labelLogo->setPixmap(pix.scaled(400,200,Qt::KeepAspectRatio));
    }

    connect(ui->actionAbout,SIGNAL(triggered()),this,SLOT(pressAbout()));
}

Resizer::~Resizer()
{
    delete ui;
}

void Resizer::setRatioMode(bool ratioMode)
{
    ui->groupSize->setChecked(!ratioMode);
}

void Resizer::setSizeMode(bool sizeMode)
{
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
    QProgressDialog *diag = new QProgressDialog(tr("Loading"),tr("Cancel"),0,paths.size()-1,this);
    if(paths.size()>1)
        diag->show();

    for(int i=0;i<paths.size();i++){
        QString filepath = paths[i];

        QFileInfo fi(filepath);

        if(fi.isDir()){
            QDir dir(filepath);
            QStringList filenames = dir.entryList(QStringList() << "*.jpg" << "*.JPG"<< "*.jpeg" << "*.JPEG" << "*.png" << "*.PNG");

            QStringList absoluteFilepaths;
            for(int i=0;i<filenames.size();i++)
                absoluteFilepaths << dir.absolutePath() + QDir::separator() + filenames.at(i);
            addList(absoluteFilepaths);

            continue;
        }


        QImage imageQt(filepath);
        QImage smallQt = imageQt.scaled(320,320,Qt::KeepAspectRatio);

        bool rotateNeeded = ui->checkRotate->isChecked();

        QTransform transform;
        if(rotateNeeded){
            int orientation = readOrientation(filepath);

            switch(orientation){
            case 6: transform.rotate(90); break;
            case 3: transform.rotate(180); break;
            case 8: transform.rotate(270); break;
            default: rotateNeeded = false;
            }
        }

        QPixmap pix;
        if(rotateNeeded){
            QImage rotatedQt = smallQt.transformed(transform);
            pix = QPixmap::fromImage(rotatedQt);
        }else{
            pix = QPixmap::fromImage(smallQt);
        }

        QLabel *label = new QLabel;
        label->setPixmap(pix);

        Image image;
        //image.original = img;
        image.original = imageQt;
        image.preview = pix;
        image.folder = fi.absoluteDir().path();
        image.filename = fi.fileName();
        image.fileinfo = fi;

        int k = mapImages.size();

        ui->gridLayout_4->addWidget(label,k/5,k%5,Qt::AlignHCenter);

        mapImages.insert(fi.absoluteFilePath(),image);

        files.push_back(fi.absoluteFilePath());


        diag->setValue( diag->value()+1 );
    }
    diag->close();
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

void Resizer::repaintGrid()
{
    int k =0;
    foreach(QString file, files){
        Image img = mapImages[file];


        QLabel *label = new QLabel;
        label->setPixmap( img.preview );

        ui->gridLayout_4->addWidget(label,k/5,k%5,Qt::AlignHCenter);

        k++;
    }
}

int Resizer::readOrientation(QString filepath)
{
    if(QFile::exists(filepath)){
        return QExifImageHeader(filepath).value(QExifImageHeader::Orientation).toShort();
    }
    return 0;
    /*
    if(QExifImageHeader(fileName).value(QExifImageHeader::Orientation).toShort() == 6)
        thumbRotation = 90;
    else if(QExifImageHeader(fileName).value(QExifImageHeader::Orientation).toShort() == 3)
        thumbRotation = 180;
    else if(QExifImageHeader(fileName).value(QExifImageHeader::Orientation).toShort() == 8)
        thumbRotation = 270;
    else
        thumbRotation = 0;*/

    /*
    Exiv2::Image::AutoPtr image = Exiv2::ImageFactory::open(filepath.toStdString().c_str());
    if(image.get() == 0)
        return 0;

    image->readMetadata();

    Exiv2::ExifData &exifData = image->exifData();
    if (exifData.empty()) {
      std::string error(filepath.toStdString());
      error += ": No Exif data found in the file";
      return 0;
    }
    Exiv2::ExifData::const_iterator end = exifData.end();

    int mode = 0;
    for (Exiv2::ExifData::const_iterator im = exifData.begin(); im != end; ++im) {
      if(im->key() == "Exif.Image.Orientation"){
        mode = im->value().toLong();
      }
    }*/
}

void Resizer::openLogo()
{
    logoPath = QFileDialog::getOpenFileName(this,tr("Select Logo"),"",tr("Image files (*.jpg *.jpeg *.png)"));

    if(logoPath.isEmpty()){
        ui->labelLogo->setText(tr("Logo"));
    }else{
        QPixmap pix(logoPath);
        ui->labelLogo->setPixmap(pix.scaled(400,200,Qt::KeepAspectRatio));
    }
}

void Resizer::resizeAll()
{

    bool ok;
    int maxSize = ui->comboPixels->currentText().toInt(&ok);
    if(!ui->checkNotResize->isChecked() && ui->groupSize->isChecked() && !ok){
        //TODO, display error
        return;
    }

    QStringList files = mapImages.keys();


    QProgressDialog *diag = new QProgressDialog(tr("Progress"),tr("Cancel"),0,files.size()-1,this);
    diag->show();


    foreach(QString file, files){
        Image img = mapImages[file];

        /*QExifImageHeader exif(img.fileinfo.absoluteFilePath());

        QList<QExifImageHeader::ImageTag> list1 = exif.imageTags();
        QList<QExifImageHeader::ExifExtendedTag> list2 = exif.extendedTags();
        QList<QExifImageHeader::GpsTag> list3 = exif.gpsTags();

        for(int i=0;i<list1.size();i++){
            qDebug() << exif.value(list1[i]).toString();
        }
        for(int i=0;i<list2.size();i++){
            qDebug() << exif.value(list2[i]).toString();
        }
        for(int i=0;i<list3.size();i++){
            qDebug() << exif.value(list3[i]).toString();
        }*/


        QString output = img.folder + QDir::separator() + "Small" + QDir::separator() + img.filename;

        QDir dir(img.folder);
        dir.mkpath(img.folder + QDir::separator() + "Small");

        //cv::Mat small;
        QImage small;

        if(ui->checkNotResize->isChecked()){
            small = img.original;
        }else{
            if(ui->groupSize->isChecked()){
                //small = createSmall(img.original,maxSize);
                small = img.original.scaled(maxSize,maxSize,Qt::KeepAspectRatio);
            }else{
                int dimMax = std::max(img.original.width(),img.original.height());
                int newSize = (double) dimMax * ui->comboRatio->currentText().toInt() / 100.0;
                //small = createSmall(img.original, (double) dimMax * ui->comboRatio->currentText().toInt() / 100.0 );
                small = img.original.scaled(newSize,newSize,Qt::KeepAspectRatio);
            }
        }

        bool rotateNeeded = ui->checkRotate->isChecked();

        QTransform transform;
        if(rotateNeeded){
            int orientation = readOrientation(img.fileinfo.absoluteFilePath());

            switch(orientation){
            case 6: transform.rotate(90); break;
            case 3: transform.rotate(180); break;
            case 8: transform.rotate(270); break;
            default: rotateNeeded = false;
            }
            small = small.transformed(transform);

        }

        if(ui->groupLogo->isChecked() && !logoPath.isEmpty() && QFile::exists(logoPath)){
            bool ok_X,ok_Y;

            int posX = ui->posXLineEdit->text().toInt(&ok_X);
            int posY = ui->posXLineEdit->text().toInt(&ok_Y);



            if(ok_X && ok_Y){
                QImage logo(logoPath);

                if(ui->radioBottomRight->isChecked()){
                    posX = small.width() - logo.width() - posX;
                    posY = small.height() - logo.height() - posY;
                }

                QPainter painter(&small);
                painter.drawImage(posX,posY,logo);
                painter.end();
            }
        }

        small.save(output);



        //exif.setValue(QExifImageHeader::Orientation,0);
        //exif.setValue(QExifImageHeader::ImageWidth,small.width());
        //exif.setValue(QExifImageHeader::ImageLength,small.height());

        //qDebug() << exif.saveToJpeg(output);


        //cv::imwrite(output.toStdString(),small);
        qDebug() << output;

        diag->setValue( diag->value()+1 );
    }

    diag->close();
    this->close();
}

void Resizer::pressAbout()
{
    QMessageBox *mess = new QMessageBox(this);
    mess->setWindowTitle(tr("About"));
    mess->setText(tr("Written by %1 (%2)\nVersion: %3","author, year, version").arg(QString::fromUtf8("Léo Baudouin"),"2013",VERSION));
    mess->setIcon(QMessageBox::Information);
    mess->exec();
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
