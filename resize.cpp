#include "resize.h"
#include "ui_resize.h"

Resize::Resize(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::Resize)
{
    ui->setupUi(this);

    /*QImage img("/home/killo/Images/test.jpg");
    QStringList tags = img.textKeys();
    qDebug() << tags.size();
    for(int i=0;i<tags.size();i++){
        qDebug() << tags.at(i) << " " << img.text( tags.at(i) );
    }

    QImageWriter writer;
    writer.setFormat("jpg");
    if (writer.supportsOption(QImageIOHandler::Description))
        qDebug() << "Jpg supports embedded text";
    else
        qDebug() << "Jpg do not supports embedded text";

    QLabel *label = new QLabel;
    label->setPixmap( QPixmap::fromImage(img).scaledToWidth(1024) );
    label->show();*/

    QStringList listSize;
    listSize << "320" << "480" << "640" << "720" << "800" << "1024" << "1280" << "2048" << "4096";
    ui->comboPixels->addItems( listSize );
    ui->comboPixels->setCurrentIndex(5);
    QStringList listRatio;
    listRatio << "10" << "20" << "30" << "33" << "40" << "50" << "60" << "66" << "70" << "80" << "90";
    ui->comboRatio->addItems( listRatio );
    ui->comboRatio->setCurrentIndex(3);



    connect(ui->buttonOpenFolder,SIGNAL(pressed()),this,SLOT(pressOpenFolder()));
    connect(ui->buttonOpenFiles,SIGNAL(pressed()),this,SLOT(pressOpenFiles()));
    connect(ui->actionAdd_folder,SIGNAL(triggered()),this,SLOT(pressOpenFolder()));
    connect(ui->actionAdd_files,SIGNAL(triggered()),this,SLOT(pressOpenFiles()));

    connect(ui->comboPixels,SIGNAL(currentIndexChanged(QString)),this,SLOT(comboPixels(QString)));
    connect(ui->comboRatio,SIGNAL(currentIndexChanged(QString)),this,SLOT(comboRatio(QString)));

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


    if(QFile::exists(QDir::homePath() + "/Images/logo.png")){
        logoPath = QDir::homePath() + "/Images/logo.png";
        //qDebug() << logoPath;
        QPixmap pix(logoPath);
        ui->labelLogo->setPixmap(pix.scaled(400,200,Qt::KeepAspectRatio));
    }

    connect(ui->actionAbout,SIGNAL(triggered()),this,SLOT(pressAbout()));
}

Resize::~Resize()
{
    delete ui;
}

void Resize::setRatioMode(bool ratioMode)
{
    ui->groupSize->setChecked(!ratioMode);
}

void Resize::setSizeMode(bool sizeMode)
{
    ui->groupRatio->setChecked(!sizeMode);
}

void Resize::pressOpenFolder()
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

void Resize::pressOpenFiles()
{
    QStringList paths = QFileDialog::getOpenFileNames(this,tr("Select Files"),QDir::homePath(), tr("Image files (*.jpg *.jpeg)"));
    if(paths.isEmpty()) return;
    QStringList absoluteFilepaths;
    for(int i=0;i<paths.size();i++){
        QFileInfo fi(paths[i]);
        absoluteFilepaths << fi.absoluteFilePath();
    }
    addList(absoluteFilepaths);
}

void Resize::editPixels(QString str)
{
    /*ui->radioEditPixels->setChecked(true);
    bool ok = false;
    str.toInt(&ok);
    if(!ok){
        QPalette palette;
        palette.setColor(QPalette::Text, Qt::red);
        ui->editPixels->setPalette(palette);
    }else{
        QPalette palette;
        palette.setColor(QPalette::Text, Qt::black);
        ui->editPixels->setPalette(palette);
    }*/
}

void Resize::addList(QStringList paths)
{
    QProgressDialog *diag = new QProgressDialog(tr("Loading"),tr("Cancel"),0,paths.size()-1,this);
    if(paths.size()>1)
        diag->show();

    for(int i=0;i<paths.size();i++){
        QString filepath = paths[i];

        QFileInfo fi(filepath);

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


/*

        cv::Mat img = cv::imread(filepath.toStdString());
        cv::Mat icon = createSmall(img,320);

        int orientation = readOrientation(filepath);
        rotate(icon,orientation);

        cv::imwrite("/tmp/tmp.jpg",icon);
        QPixmap pix("/tmp/tmp.jpg");*/

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

void Resize::addFile(QString filepath)
{
    addList(QStringList() << filepath);
}

void Resize::removeFile(QString filepath)
{
    QFileInfo fi(filepath);

    mapImages.remove(fi.absoluteFilePath());

    repaintGrid();

}

void Resize::repaintGrid()
{
    //delete ui->gridLayout_4;

    //ui->gridLayout_4 = new QGridLayout(ui->scrollAreaWidgetContents);



    int k =0;
    foreach(QString file, files){
        Image img = mapImages[file];


        QLabel *label = new QLabel;
        label->setPixmap( img.preview );

        ui->gridLayout_4->addWidget(label,k/5,k%5,Qt::AlignHCenter);

        k++;
    }
}

void Resize::editRatio(QString str)
{
    /*ui->radioEditRatio->setChecked(true);
    bool ok = false;
    str.toInt(&ok);
    if(!ok){
        QPalette palette;
        palette.setColor(QPalette::Text, Qt::red);
        ui->editRatio->setPalette(palette);
    }else{
        QPalette palette;
        palette.setColor(QPalette::Text, Qt::black);
        ui->editRatio->setPalette(palette);
    }*/
}

void Resize::comboPixels(QString)
{
    //ui->radioComboPixels->setChecked(true);
}

void Resize::comboRatio(QString)
{
    //ui->radioComboRatio->setChecked(true);
}

int Resize::readOrientation(QString filepath)
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
/*
void Resize::rotate(cv::Mat &img, int mode)
{
    switch(mode){
    case 1:{
        // 0°
        break;
    }
    case 6:{
        // 90°
        cv::transpose(img,img);
        break;
    }
    case 3:{
        // 180°
        cv::flip(img,img,-1);
        cv::flip(img,img,1);
        break;
    }
    case 8:{
        // 270°
        cv::transpose(img,img);
        cv::flip(img,img,-1);
        cv::flip(img,img,1);
        break;
    }
    default :
      break;
    }
}
*/
/*
cv::Mat Resize::createSmall(cv::Mat &img, int size)
{
    int height = img.rows;
    int width  = img.cols;

    int smallWidth;
    int smallHeight;

    if(width>height){
      smallWidth = size;
      smallHeight = size * height / width;
    }else{
      smallWidth =  size * width / height;
      smallHeight = size;
    }

    cv::Mat small;// = cv::Mat(smallHeight,smallWidth,img.type());
    //cv::resize(img,small,cv::Size(smallHeight,smallWidth));
    cv::resize(img,small,cv::Size(smallWidth,smallHeight));


    return small;
}
*/
void Resize::openLogo()
{
    logoPath = QFileDialog::getOpenFileName(this,tr("Select Logo"),"",tr("Image files (*.jpg *.jpeg *.png)"));

    if(logoPath.isEmpty()){
        ui->labelLogo->setText(tr("Logo"));
    }else{
        QPixmap pix(logoPath);
        ui->labelLogo->setPixmap(pix.scaled(400,200,Qt::KeepAspectRatio));
    }
}
/*
void Resize::addLogo(cv::Mat &img, cv::Mat &logo)
{
  IplImage *logo = cvLoadImage("/var/local/logo_lulu.png");

  int offsetx = 400;
  int offsety = 100;

  for(int i=0;i<logo->width;i++){
    for(int j=0;j<logo->height;j++){
      int x = img->width-offsetx+i;
      int y = img->height-offsety+j;
      CvScalar piximg = cvGet2D(img,y,x);
      CvScalar pixlogo = cvGet2D(logo,j,i);
      int minimal1 = 25;
      int minimal2 = 45;
      double val = pixlogo.val[0];
      if(val>minimal1){
    CvScalar pix;
    pix.val[0] = 31;
    pix.val[1] = 45;
    pix.val[2] = 93;
    for(int k=0;k<3;k++)
      pix.val[k] *= 0.75;

    if(val<minimal2){
      for(int k=0;k<3;k++)
        pix.val[k] = 0.5 * (pix.val[k] + piximg.val[k]);
    }
    cvSet2D(img,y,x,pix);
      }
    }
  }
  cvReleaseImage(&logo);
}*/

void Resize::resizeAll()
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

void Resize::pressAbout()
{
    QMessageBox *mess = new QMessageBox(this);
    mess->setWindowTitle(tr("About"));
    mess->setText(tr("Written by %1 (%2)\nVersion: %3","author, year, version").arg(QString::fromUtf8("Léo Baudouin"),"2013",VERSION));
    mess->setIcon(QMessageBox::Information);
    mess->exec();
}




void Resize::handleMessage(const QString& message)
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
        addFile(filename);
        emit needToShow();
    }
    break;
    default:
    break;
    };
}
