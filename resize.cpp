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
    listSize << "320" << "480" << "600" << "720" << "800" << "1024" << "1280" << "2048" << "4096";
    ui->comboPixels->addItems( listSize );
    ui->comboPixels->setCurrentIndex(5);
    QStringList listRatio;
    listRatio << "10" << "20" << "30" << "33" << "40" << "50" << "60" << "66" << "70" << "80" << "90";
    ui->comboRatio->addItems( listRatio );
    ui->comboRatio->setCurrentIndex(3);


    connect(ui->buttonOpenFolder,SIGNAL(pressed()),this,SLOT(pressOpenFolder()));
    connect(ui->buttonOpenFiles,SIGNAL(pressed()),this,SLOT(pressOpenFiles()));
    connect(ui->comboPixels,SIGNAL(currentIndexChanged(QString)),this,SLOT(comboPixels(QString)));
    connect(ui->comboRatio,SIGNAL(currentIndexChanged(QString)),this,SLOT(comboRatio(QString)));

    connect(ui->groupRatio,SIGNAL(clicked(bool)),this,SLOT(setRatioMode(bool)));
    connect(ui->groupSize,SIGNAL(clicked(bool)),this,SLOT(setSizeMode(bool)));

    /*QButtonGroup *buttonGroup1 = new QButtonGroup;
    buttonGroup1->addButton(ui->groupRatio);
    buttonGroup1->addButton(ui->groupSize);
    buttonGroup1->setExclusive(true);*/

    QFileIconProvider icons;
    ui->buttonOpenFiles->setText("");
    ui->buttonOpenFolder->setText("");
    ui->buttonOpenFiles->setIcon(icons.icon(QFileIconProvider::File));
    ui->buttonOpenFolder->setIcon(icons.icon(QFileIconProvider::Folder));

    connect(ui->buttonBox,SIGNAL(rejected()),this,SLOT(close()));
    connect(ui->buttonBox,SIGNAL(accepted()),this,SLOT(resizeAll()));
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
    for(int i=0;i<paths.size();i++){
        QString filepath = paths[i];

        QFileInfo fi(filepath);

        cv::Mat img = cv::imread(filepath.toStdString());
        cv::Mat icon = createSmall(img,320);

        int orientation = readOrientation(filepath);
        rotate(icon,orientation);

        cv::imwrite("/tmp/tmp.jpg",icon);
        QPixmap pix("/tmp/tmp.jpg");

        QLabel *label = new QLabel;
        label->setPixmap(pix);

        Image image;
        image.original = img;
        image.preview = pix;
        image.folder = fi.absoluteDir().path();
        image.filename = fi.fileName();

        int k = mapImages.size();

        ui->gridLayout_4->addWidget(label,k/5,k%5,Qt::AlignHCenter);

        mapImages.insert(fi.absoluteFilePath(),image);

        files.push_back(fi.absoluteFilePath());
    }
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
    }
}

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

void Resize::openLogo()
{
    logoPath = QFileDialog::getOpenFileName(this,tr("Select Logo"),"",tr("Image files (*.jpg *.jpeg *.png)"));
}

void Resize::resizeAll()
{
    bool ok;
    int maxSize = ui->comboPixels->currentText().toInt(&ok);
    if(ui->groupSize->isChecked() && !ok){
        //TODO, display error
        return;
    }

    QStringList files = mapImages.keys();

    foreach(QString file, files){
        Image img = mapImages[file];

        QString output = img.folder + QDir::separator() + "Small" + QDir::separator() + img.filename;

        QDir dir(img.folder);
        dir.mkpath(img.folder + QDir::separator() + "Small");

        cv::Mat small;
        if(ui->groupSize->isChecked()){
            small = createSmall(img.original,maxSize);
        }else{
            int dimMax = std::max(img.original.rows,img.original.cols);
            small = createSmall(img.original, (double) dimMax * ui->comboRatio->currentText().toInt() / 100.0 );
        }

        cv::imwrite(output.toStdString(),small);
        qDebug() << output;
    }

    this->close();
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
