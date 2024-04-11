/**
   @author Kenta Suzuki
*/

#include "rqt_minio_client/mainwindow.h"

#include <QAction>
#include <QBoxLayout>
#include <QCalendarWidget>
#include <QCheckBox>
#include <QComboBox>
#include <QDateTime>
#include <QDialog>
#include <QDialogButtonBox>
#include <QDir>
#include <QFile>
#include <QFileDialog>
#include <QFileInfo>
#include <QFileSystemWatcher>
#include <QFormLayout>
#include <QGridLayout>
#include <QHBoxLayout>
#include <QJsonArray>
#include <QJsonDocument>
#include <QJsonObject>
#include <QLabel>
#include <QLineEdit>
#include <QListWidget>
#include <QListWidgetItem>
#include <QMap>
#include <QMessageBox>
#include <QMenu>
#include <QMenuBar>
#include <QMimeType>
#include <QMimeDatabase>
#include <QProcess>
#include <QProgressDialog>
#include <QPushButton>
#include <QSettings>
#include <QStatusBar>
#include <QSystemTrayIcon>
#include <QTextStream>
#include <QToolBar>
#include <QTreeWidget>

#include "rqt_minio_client/bucket_item.h"
#include "rqt_minio_client/json_archive.h"

namespace {

Aws::Vector<Aws::S3::Model::Tag> objectTags;

}

namespace rqt_minio_client {

struct Credentials {
    QString accessKeyName;
    QString secretKeyName;
    QString endpointName;

    Aws::Client::ClientConfiguration* clientConfig;

    bool ready()
    {
        if(!accessKeyName.isEmpty() && !secretKeyName.isEmpty() && !endpointName.isEmpty()) {
            return true;
        }
        return false;
    }
};

class CreateBucketDialog : public QDialog
{
public:
    CreateBucketDialog(QWidget* parent = nullptr);

    QString bucketName() const { return bucketLine->text(); }

private:

    QLineEdit* bucketLine;
    QDialogButtonBox* buttonBox;
};

class TaggingDialog : public QDialog, public JsonArchive
{
public:
    TaggingDialog(QWidget* parent = nullptr);

    void setBucketName(const QString& bucketName) { this->bucketName = bucketName; }
    void setCredentials(const Credentials& credentials) { this->credentials = credentials; }

private:
    void open();
    void save();

    void putObject(const QString& fileName);
    void putObjects(const QString& fileName);
    bool completed() const;

    void on_resetButton_clicked();
    void on_putFileButton_clicked();
    void on_putDirButton_clicked();
    void on_calendarButton_clicked();

    void createMenu();
    
    virtual void read(const QJsonObject& json) override;
    virtual void write(QJsonObject& json) override;

    QMenuBar* menuBar;
    QLineEdit* pathLine;
    QLineEdit* keyLines[10];
    QLineEdit* valueLines[10];
    QString bucketName;
    QString dirName;
    QDialogButtonBox* buttonBox;

    QMenu* fileMenu;
    QAction* openAct;
    QAction* saveAct;
    QAction* exitAct;

    Credentials credentials;
};

class HeadDialog : public QDialog
{
public:
    HeadDialog(QWidget* parent = nullptr);

    void setBucketName(const QString& bucketName) { this->bucketName = bucketName; }
    void setCredentials(const Credentials& credentials) { this->credentials = credentials; }
    void setObjectKeyList(const QStringList& objectKeyList) { this->objectKeyList = objectKeyList; }
    void setTaggingKeyList(const QStringList& taggingKeyList) { this->taggingKeyList = taggingKeyList; }
    void setValueMap(const QMap<QString, QStringList>& valueMap) { this->valueMap = valueMap; }
    QStringList keyList() const { return this->headKeyList; }
    QString text() const { return this->message; }
    void update();

private:
    void head();

    void on_resetButton_clicked();
    void on_keyCombo_currentIndexChanged(int arg1);
    void on_beginButton_clicked();
    void on_endButton_clicked();

    QStringList objectKeyList;
    QStringList headKeyList;
    QStringList taggingKeyList;
    QMap<QString, QStringList> valueMap;
    QComboBox* typeCombo;
    QComboBox* keyCombo[10];
    QComboBox* valueCombo[10];
    QLineEdit* beginLine;
    QLineEdit* endLine;
    QDate beginDate;
    QDate endDate;
    QString bucketName;
    QString message;
    QDialogButtonBox* buttonBox;

    Credentials credentials;
};

class InfoDialog : public QDialog
{
public:
    InfoDialog(QWidget* parent = nullptr);

    void setBucketName(const QString& bucketName) { this->bucketName = bucketName; }
    void setCredentials(const Credentials& credentials) { this->credentials = credentials; }
    void setObjectKey(const QString& objectKeyName) { this->objectKeyName = objectKeyName; }
    void update();

private:

    QLabel* nameLabel;
    QLabel* sizeLabel;
    QLabel* lastModifiedLabel;
    QLabel* eTagLabel;
    QTreeWidget* tagTree;
    QString bucketName;
    QString objectKeyName;

    Credentials credentials;
};

class EditDialog : public QDialog
{
public:
    EditDialog(QWidget* parent = nullptr);

    void setBucketName(const QString& bucketName) { this->bucketName = bucketName; }
    void setCredentials(const Credentials& credentials) { this->credentials = credentials; }
    void setObjectKey(const QString& objectKeyName) { this->objectKeyName = objectKeyName; }
    void update();

private:
    void overwrite();
    void tagging();

    void on_resetButton_clicked();

    QLineEdit* keyLines[10];
    QLineEdit* valueLines[10];
    QString bucketName;
    QString objectKeyName;
    QDialogButtonBox* buttonBox;

    Credentials credentials;
};

class CredentialDialog : public QDialog, public JsonArchive
{
public:
    CredentialDialog(QWidget* parent = nullptr);

    void setAccessKeyId(const QString& accessKeyName) { accessKeyIdLine->setText(accessKeyName); }
    QString accessKeyId() const { return accessKeyIdLine->text(); }
    void setSecretKey(const QString& secretKeyName) { secretKeyLine->setText(secretKeyName); }
    QString secretKey() const { return secretKeyLine->text(); }
    void setEndpoint(const QString& endpointName) { endpointLine->setText(endpointName); }
    QString endpoint() const { return endpointLine->text(); }

private:
    void open();
    void save();

    void createMenu();

    virtual void read(const QJsonObject& json) override;
    virtual void write(QJsonObject& json) override;

    QMenuBar* menuBar;
    QLineEdit* accessKeyIdLine;
    QLineEdit* secretKeyLine;
    QLineEdit* endpointLine;
    QDialogButtonBox* buttonBox;

    QMenu* fileMenu;
    QAction* openAct;
    QAction* saveAct;
    QAction* exitAct;
};

class CalendarDialog : public QDialog
{
public:
    CalendarDialog(QWidget* parent = nullptr);

    void setMaximumDate(QDate date) { calendar->setMaximumDate(date); }
    void setMinimumDate(QDate date) { calendar->setMinimumDate(date); }
    QDate selectedDate() const { return calendar->selectedDate(); }

private:

    QCalendarWidget* calendar;
    QDialogButtonBox* buttonBox;
};

class WatcherWidget : public QWidget
{
public:
    WatcherWidget(QWidget* parent = nullptr);

    void add();
    
    void on_systemWatcher_directoryChanged(const QString& path);
    void on_fileList_customContextMenuRequested(const QPoint& pos);

    void createActions();

    QAction* addAct;
    QAction* showAct;

    QListWidget* fileList;
    QFileSystemWatcher* systemWatcher;
};

class MainWindow::Impl
{
public:
    MainWindow* self;

    Impl(MainWindow* self);
    ~Impl();

    void listBucket();
    void createBucket();
    void delBucket();
    void policy();
    void list();
    void newBucket(const QString& bucketName);
    void del();

    void delObjects();
    void headObjects();    
    void getObjects();
    void taggingObjects();
    void filterObjects();
    void countObjects();
    void update();

    void info();
    void edit();
    void check();
    void uncheck();
    void listItems();
    void delItems();
    void getItems(const QString& fileName);
    void clearItems();
    bool checkedAny();

    void login();
    void browse();
    bool ready();
    void accept();

    void on_systemWatcher_directoryChanged(const QString& path);
    void on_objectList_customContextMenuRequested(const QPoint& pos);

    void createActions();
    void createMenus();
    void createToolBars();

    void writeSettings();
    void readSettings();

    QAction* listBucketAct;
    QAction* createBucketAct;
    QAction* delBucketAct;
    QAction* listObjectAct;
    QAction* putObjectAct;
    QAction* delObjectAct;
    QAction* headObjectAct;
    QAction* getObjectAct;
    QAction* infoAct;
    QAction* editAct;
    QAction* checkAct;
    QAction* uncheckAct;
    QAction* loginAct;
    QAction* browseAct;
    QAction* watchAct;
    QAction* noteAct;

    QComboBox* bucketCombo;
    QComboBox* typeCombo;
    QLineEdit* filterLine;
    QListWidget* objectList;
    QStringList objectKeyList;
    QStringList headKeyList;
    QStringList taggingKeyList;
    QMap<QString, QStringList> valueMap;
    QSystemTrayIcon* trayIcon;
    QString endpointName;
    QString accessKeyName;
    QString secretKeyName;

    WatcherWidget* watcherWidget;

    bool is_ready;

    Aws::Client::ClientConfiguration* clientConfig;
    Aws::SDKOptions options;
};

MainWindow::MainWindow(QWidget* parent)
    : QMainWindow(parent)
{
    impl = new Impl(this);
}

MainWindow::Impl::Impl(MainWindow* self)
    : self(self)
{
    QWidget* widget = new QWidget;
    self->setCentralWidget(widget);

    Aws::InitAPI(options);

    createActions();
    createMenus();
    createToolBars();

    self->setWindowTitle("MinIO Client");

    is_ready = false;
    clientConfig = nullptr;

    filterLine = new QLineEdit;
    filterLine->setPlaceholderText("Start typing to filter objects in the bucket");
    self->connect(filterLine, QOverload<const QString&>::of(&QLineEdit::textEdited),
        [=](QString text){ filterObjects(); });
    
    objectList = new QListWidget;
    objectList->setContextMenuPolicy(Qt::CustomContextMenu);
    self->connect(objectList, QOverload<QListWidgetItem*>::of(&QListWidget::itemClicked),
        [=](QListWidgetItem* item){ countObjects(); });
    self->connect(objectList, &QListWidget::customContextMenuRequested,
        [&](QPoint pos){ on_objectList_customContextMenuRequested(pos); });

    watcherWidget = new WatcherWidget;
    self->connect(watcherWidget->systemWatcher, &QFileSystemWatcher::directoryChanged,
        [&](QString path){ on_systemWatcher_directoryChanged(path); });

    QVBoxLayout* layout = new QVBoxLayout;
    layout->addWidget(objectList);
    layout->addWidget(filterLine);
    widget->setLayout(layout);

    countObjects();
    readSettings();
}

MainWindow::~MainWindow()
{
    delete impl;
}

MainWindow::Impl::~Impl()
{
    Aws::ShutdownAPI(options);
}

void MainWindow::Impl::listBucket()
{
    if(!ready()) {
        return;
    }

    bucketCombo->blockSignals(true);
    bucketCombo->clear();
    bucketCombo->addItem("");
    Aws::Vector<Aws::String> buckets = BucketItem::getBuckets(accessKeyName.toStdString(), secretKeyName.toStdString(), *clientConfig);

    QProgressDialog progress("Getting buckets...", "Cancel", 0, buckets.size(), self);
    progress.setWindowModality(Qt::WindowModal);

    for(int i = 0; i < buckets.size(); ++i) {
        progress.setValue(i);
        if(progress.wasCanceled()) {
            break;
        }
        bucketCombo->addItem(buckets[i].c_str());
    }
    progress.setValue(buckets.size());
    bucketCombo->blockSignals(false);
}

void MainWindow::Impl::createBucket()
{
    CreateBucketDialog dialog(self);

    if(dialog.exec()) {
        QString bucketName = dialog.bucketName();
        newBucket(bucketName);
    }
}

void MainWindow::Impl::delBucket()
{
    QString bucketName = bucketCombo->currentText();
    if(bucketName.isEmpty()) {
        return;
    }
    QString text = "Do you want to delete \n\""
        + bucketName +  "\"?";

    QMessageBox msgBox;
    msgBox.setText(text);
    msgBox.setStandardButtons(QMessageBox::Ok | QMessageBox::Cancel);
    msgBox.setDefaultButton(QMessageBox::Ok);
    int ret = msgBox.exec();

    switch(ret) {
    case QMessageBox::Ok:
        // Ok was clicked
        del();
        break;
    case QMessageBox::Cancel:
        // Cancel was clicked
        break;
    default:
        // should never be reached
        break;
    }
}

void MainWindow::Impl::policy()
{
    if(!ready()) {
        return;
    }

    QString bucketName = bucketCombo->currentText();
    if(!bucketName.isEmpty()) {
        BucketItem* bucketItem = new BucketItem(bucketName.toStdString());
        bucketItem->createCredentials(accessKeyName.toStdString(), secretKeyName.toStdString());
        bucketItem->getBucketPolicy(*clientConfig);
    }
}

void MainWindow::Impl::list()
{
    listBucket();

    int ret = QMessageBox::information(self, "Information",
        "Select a bucket.", QMessageBox::Ok, QMessageBox::Ok);
}

void MainWindow::Impl::newBucket(const QString& bucketName)
{
    if(!ready()) {
        return;
    }

    if(!bucketName.isEmpty()) {
        BucketItem* bucketItem = new BucketItem(bucketName.toStdString());
        bucketItem->createCredentials(accessKeyName.toStdString(), secretKeyName.toStdString());
        bucketItem->createBucket(*clientConfig);
    }

    listBucket();
    bucketCombo->setCurrentText(bucketName);
}

void MainWindow::Impl::del()
{
    if(!ready()) {
        return;
    }

    QString bucketName = bucketCombo->currentText();
    if(!bucketName.isEmpty()) {
        BucketItem* bucketItem = new BucketItem(bucketName.toStdString());
        bucketItem->createCredentials(accessKeyName.toStdString(), secretKeyName.toStdString());
        bucketItem->deleteBucket(*clientConfig);
        listBucket();
    }
}

void MainWindow::Impl::delObjects()
{
    if(!checkedAny()) {
        return;
    }

    QString text = "Do you want to delete the checked objects?";

    QMessageBox msgBox;
    msgBox.setText(text);
    msgBox.setStandardButtons(QMessageBox::Ok | QMessageBox::Cancel);
    msgBox.setDefaultButton(QMessageBox::Ok);
    int ret = msgBox.exec();

    switch(ret) {
    case QMessageBox::Ok:
        // Ok was clicked
        delItems();
        break;
    case QMessageBox::Cancel:
        // Cancel was clicked
        break;
    default:
        // should never be reached
        break;
    }
}

void MainWindow::Impl::headObjects()
{
    HeadDialog dialog(self);
    Credentials credentials = { accessKeyName, secretKeyName, endpointName, clientConfig };
    dialog.setCredentials(credentials);
    dialog.setBucketName(bucketCombo->currentText());
    dialog.setObjectKeyList(objectKeyList);
    dialog.setTaggingKeyList(taggingKeyList);
    dialog.setValueMap(valueMap);
    dialog.update();

    if(dialog.exec()) {
        objectList->clear();
        headKeyList = dialog.keyList();
        for(int i = 0; i < headKeyList.size(); ++i) {
            QListWidgetItem* item = new QListWidgetItem(objectList);
            item->setText(headKeyList.at(i));
            item->setCheckState(Qt::Unchecked);
        }
        filterObjects();
        self->statusBar()->showMessage(dialog.text());
    }
}

void MainWindow::Impl::getObjects()
{
    if(!checkedAny()) {
        return;
    }

    static QString dir2 = "/home";
    QString dir = QFileDialog::getExistingDirectory(self, "Open Directory",
        dir2,
        QFileDialog::ShowDirsOnly
        | QFileDialog::DontResolveSymlinks);

    if(dir.isEmpty()) {
        return;
    } else {
        dir2 = dir;
        getItems(dir);
    }
}

void MainWindow::Impl::taggingObjects()
{
    TaggingDialog dialog(self);
    Credentials credentials = { accessKeyName, secretKeyName, endpointName, clientConfig };
    dialog.setCredentials(credentials);
    dialog.setBucketName(bucketCombo->currentText());

    if(dialog.exec()) {
        listItems();
    }
}

void MainWindow::Impl::filterObjects()
{
    QStringList list;
    QMimeDatabase database;
    QString type = typeCombo->currentText();
    QString filter = filterLine->text();

    if(headKeyList.size() == 0) {
        for(int i = 0; i < objectList->count(); ++i) {
            QListWidgetItem* item = objectList->item(i);
            headKeyList << item->text();
        }
    }

    for(int i = 0; i < headKeyList.size(); ++i) {
        QString objectKeyName = headKeyList.at(i);
        QMimeType mimeType = database.mimeTypeForFile(objectKeyName);
        QString name = mimeType.name();

        if(type == name || type == "") {
            if(!filter.isEmpty()) {
                if(objectKeyName.contains(filter)) {
                    list << objectKeyName;
                }
            } else {
                list << objectKeyName;
            }
        }
    }

    objectList->clear();
    for(int i = 0; i < list.size(); ++i) {
        QListWidgetItem* item = new QListWidgetItem(objectList);
        item->setText(list.at(i));
        item->setCheckState(Qt::Unchecked);
    }

    countObjects();
}

void MainWindow::Impl::countObjects()
{
    int count = 0;
    for(int i = 0; i < objectList->count(); ++i) {
        QListWidgetItem* item = objectList->item(i);
        if(item->checkState() == Qt::Checked) {
            ++count;
        }
    }

    QString text = QString("%1/%2 objects have been checked")
        .arg(count).arg(objectList->count());
    self->statusBar()->showMessage(text);
}

void MainWindow::Impl::update()
{
    if(!ready()) {
        return;
    }

    objectKeyList.clear();
    headKeyList.clear();
    typeCombo->blockSignals(true);
    typeCombo->clear();
    typeCombo->addItem("");
    typeCombo->blockSignals(false);
    QMimeDatabase database;

    QString bucketName = bucketCombo->currentText();
    if(!bucketName.isEmpty()) {
        BucketItem* bucketItem = new BucketItem(bucketName.toStdString());
        bucketItem->createCredentials(accessKeyName.toStdString(), secretKeyName.toStdString());

        objectTags.clear();

        QProgressDialog progress("Listing objects...", "Cancel", 0, objectList->count(), self);
        progress.setWindowModality(Qt::WindowModal);

        for(int i = 0; i < objectList->count(); ++i) {
            progress.setValue(i);
            if(progress.wasCanceled()) {
                break;
            }
            QString objectKeyName = objectList->item(i)->text();
            objectKeyList << objectKeyName;
            Aws::Vector<Aws::S3::Model::Tag> tags = bucketItem->getObjectsTagging(objectKeyName.toStdString(), *clientConfig);
            for(int j = 0; j < tags.size(); ++j) {
                objectTags.push_back(tags[j]);
            }

            QMimeType mimeType = database.mimeTypeForFile(objectKeyName);
            QString name = mimeType.name();
            if(typeCombo->findText(name) == -1) {
                typeCombo->blockSignals(true);
                typeCombo->addItem(name);
                typeCombo->blockSignals(false);
            }
        }
        progress.setValue(objectList->count());
        
        taggingKeyList.clear();
        valueMap.clear();
        for(int i = 0; i < objectTags.size(); ++i) {
            Aws::S3::Model::Tag tag = objectTags[i];
            QString key = tag.GetKey().c_str();
            QString value = tag.GetValue().c_str();
            taggingKeyList << key;
            QStringList& list = valueMap[key];
            list << value;
            list.removeDuplicates();
        }
        taggingKeyList.removeDuplicates();
    }

    typeCombo->blockSignals(true);
    typeCombo->setCurrentIndex(0);
    typeCombo->blockSignals(false);

    filterObjects();
    self->statusBar()->showMessage("");
}

void MainWindow::Impl::info()
{
    QList<QListWidgetItem*> selectedItems = objectList->selectedItems();
    if(selectedItems.size() == 0) {
        return;
    }

    InfoDialog dialog(self);
    Credentials credentials = { accessKeyName, secretKeyName, endpointName, clientConfig };
    dialog.setCredentials(credentials);
    dialog.setBucketName(bucketCombo->currentText());
    dialog.setObjectKey(selectedItems.front()->text());
    dialog.update();

    if(dialog.exec()) {

    }
}

void MainWindow::Impl::edit()
{
    QList<QListWidgetItem*> selectedItems = objectList->selectedItems();
    if(selectedItems.size() == 0) {
        return;
    }

    EditDialog dialog(self);
    Credentials credentials = { accessKeyName, secretKeyName, endpointName, clientConfig };
    dialog.setCredentials(credentials);
    dialog.setBucketName(bucketCombo->currentText());
    dialog.setObjectKey(selectedItems.front()->text());
    dialog.update();

    if(dialog.exec()) {
        listItems();
    }
}

void MainWindow::Impl::check()
{
    for(int i = 0; i < objectList->count(); ++i) {
        QListWidgetItem* item = objectList->item(i);
        item->setCheckState(Qt::Checked);
    }
    countObjects();
}

void MainWindow::Impl::uncheck()
{
    for(int i = 0; i < objectList->count(); ++i) {
        QListWidgetItem* item = objectList->item(i);
        item->setCheckState(Qt::Unchecked);
    }
    countObjects();
}

void MainWindow::Impl::listItems()
{
    if(!ready()) {
        return;
    }

    clearItems();

    QString bucketName = bucketCombo->currentText();
    if(!bucketName.isEmpty()) {
        BucketItem* bucketItem = new BucketItem(bucketName.toStdString());
        bucketItem->createCredentials(accessKeyName.toStdString(), secretKeyName.toStdString());

        Aws::Vector<Aws::String> objects = bucketItem->getObjects(*clientConfig);

        QProgressDialog progress("Listing objects...", "Cancel", 0, objects.size(), self);
        progress.setWindowModality(Qt::WindowModal);

        for(int i = 0; i < objects.size(); ++i) {
            progress.setValue(i);
            if(progress.wasCanceled()) {
                break;
            }
            QString objectKeyName = objects[i].c_str();
            QListWidgetItem* item = new QListWidgetItem(objectList);
            item->setText(objectKeyName);
            item->setCheckState(Qt::Unchecked);
        }
        progress.setValue(objects.size());
    }

    update();
}

void MainWindow::Impl::delItems()
{
    if(!ready()) {
        return;
    }

    QString bucketName = bucketCombo->currentText();
    if(!bucketName.isEmpty()) {
        BucketItem* bucketItem = new BucketItem(bucketName.toStdString());
        bucketItem->createCredentials(accessKeyName.toStdString(), secretKeyName.toStdString());

        QProgressDialog progress("Deleting objects...", "Cancel", 0, objectList->count(), self);

        progress.setWindowModality(Qt::WindowModal);
        for(int i = 0; i < objectList->count(); ++i) {
            progress.setValue(i);
            if(progress.wasCanceled()) {
                break;
            }
            QListWidgetItem* item = objectList->item(i);
            if(item->checkState() == Qt::Checked) {
                QString objectKeyName = item->text();
                bucketItem->deleteObject(objectKeyName.toStdString(), *clientConfig);
            }
        }
        progress.setValue(objectList->count());
        listItems();
    }
}

void MainWindow::Impl::getItems(const QString& fileName)
{
    if(!ready()) {
        return;
    }

    QString bucketName = bucketCombo->currentText();
    if(!bucketName.isEmpty()) {
        BucketItem* bucketItem = new BucketItem(bucketName.toStdString());
        bucketItem->createCredentials(accessKeyName.toStdString(), secretKeyName.toStdString());

        QProgressDialog progress("Getting objects...", "Cancel", 0, objectList->count(), self);
        progress.setWindowModality(Qt::WindowModal);

        for(int i = 0; i < objectList->count(); ++i) {
            progress.setValue(i);
            if(progress.wasCanceled()) {
                break;
            }
            QListWidgetItem* item = objectList->item(i);
            if(item->checkState() == Qt::Checked) {
                QString objectKeyName = item->text();
                QStringList list = objectKeyName.split("/");
                QDir dir(fileName);
                for(int j = 0; j < list.size() - 1; ++j) {
                    QString subDirPath = list.at(j);
                    if(!dir.exists(subDirPath)) {
                        if(!dir.mkdir(subDirPath)) {

                        }
                    }
                    dir.cd(subDirPath);
                }

                QString filePath = QString("%1/%2").arg(fileName).arg(objectKeyName);
                bucketItem->getObject(objectKeyName.toStdString(), filePath.toStdString(), *clientConfig);
            }
        }
        progress.setValue(objectList->count());
    }
}

void MainWindow::Impl::clearItems()
{
    while(objectList->count() > 0) {
        objectList->takeItem(0);
    }
    countObjects();
}

bool MainWindow::Impl::checkedAny()
{
    bool is_checked = false;
    for(int i = 0; i < objectList->count(); ++i) {
        QListWidgetItem* item = objectList->item(i);
        if(item->checkState() == Qt::Checked) {
            is_checked = true;
        }
    }

    return is_checked;
}

void MainWindow::Impl::login()
{
    CredentialDialog dialog(self);
    dialog.setAccessKeyId(accessKeyName);
    dialog.setSecretKey(secretKeyName);
    dialog.setEndpoint(endpointName);

    if(dialog.exec()) {
        accessKeyName = dialog.accessKeyId();
        secretKeyName = dialog.secretKey();
        endpointName = dialog.endpoint();
        accept();
    }
}

void MainWindow::Impl::browse()
{
    QString program = "xdg-open";
    QProcess::startDetached(program, QStringList() << endpointName);
}

bool MainWindow::Impl::ready()
{
    if(!is_ready) {
        login();
    }
    return is_ready;
}

void MainWindow::Impl::accept()
{
    if(!endpointName.isEmpty()) {
        if(!clientConfig) {
            clientConfig = new Aws::Client::ClientConfiguration;
        }
        clientConfig->scheme = Aws::Http::Scheme::HTTP;
        clientConfig->endpointOverride = endpointName.toStdString().c_str();
    }

    is_ready = true;
    list();
}

void MainWindow::Impl::on_systemWatcher_directoryChanged(const QString& path)
{
    QSystemTrayIcon::MessageIcon msgIcon = QSystemTrayIcon::MessageIcon(QSystemTrayIcon::Information);

    QString title = "Information";
    QString message = "Directory has been changed";

    trayIcon->showMessage(title, message, msgIcon,
        15 * 1000);
}

void MainWindow::Impl::on_objectList_customContextMenuRequested(const QPoint& pos)
{
    QMenu menu(self);
    menu.addAction(infoAct);
    menu.addAction(editAct);
    menu.addSeparator();
    menu.addAction(checkAct);
    menu.addAction(uncheckAct);
    menu.exec(objectList->mapToGlobal(pos));
}

void MainWindow::Impl::createActions()
{
    const QIcon listIcon = QIcon::fromTheme("view-refresh");
    listBucketAct = new QAction(listIcon, "&List", self);
    listBucketAct->setStatusTip("List buckets");
    self->connect(listBucketAct, &QAction::triggered, [&](){ listBucket(); });

    const QIcon createIcon = QIcon::fromTheme("list-add");
    createBucketAct = new QAction(createIcon, "&Create", self);
    createBucketAct->setStatusTip("Create a bucket");
    self->connect(createBucketAct, &QAction::triggered, [&](){ createBucket(); });

    const QIcon deleteIcon = QIcon::fromTheme("list-remove");
    delBucketAct = new QAction(deleteIcon, "&Delete", self);
    delBucketAct->setStatusTip("Delete the bucket");
    self->connect(delBucketAct, &QAction::triggered, [&](){ delBucket(); });

    listObjectAct = new QAction(listIcon, "&List", self);
    listObjectAct->setStatusTip("List objects");
    self->connect(listObjectAct, &QAction::triggered, [&](){ listItems(); });

    const QIcon putFileIcon = QIcon::fromTheme("list-add");
    putObjectAct = new QAction(putFileIcon, "&Put", self);
    putObjectAct->setStatusTip("Put objects");
    self->connect(putObjectAct, &QAction::triggered, [&](){ taggingObjects(); });

    delObjectAct = new QAction(deleteIcon, "&Delete", self);
    delObjectAct->setStatusTip("Delete the objects");
    self->connect(delObjectAct, &QAction::triggered, [&](){ delObjects(); });

    const QIcon headIcon = QIcon::fromTheme("edit-find");
    headObjectAct = new QAction(headIcon, "&Head");
    headObjectAct->setStatusTip("Head objects");
    self->connect(headObjectAct, &QAction::triggered, [&](){ headObjects(); });

    const QIcon getIcon = QIcon::fromTheme("document-save");
    getObjectAct = new QAction(getIcon, "&Get", self);
    getObjectAct->setStatusTip("Get the objects");
    self->connect(getObjectAct, &QAction::triggered, [&](){ getObjects(); });

    infoAct = new QAction("&Info", self);
    infoAct->setStatusTip("Show the object info");
    self->connect(infoAct, &QAction::triggered, [&](){ info(); });

    editAct = new QAction("&Edit", self);
    editAct->setStatusTip("Edit the object tagging");
    self->connect(editAct, &QAction::triggered, [&](){ edit(); });

    checkAct = new QAction("&Check All", self);
    checkAct->setStatusTip("Check all objects");
    self->connect(checkAct, &QAction::triggered, [&](){ check(); });

    uncheckAct = new QAction("&Uncheck All", self);
    uncheckAct->setStatusTip("Uncheck all objects");
    self->connect(uncheckAct, &QAction::triggered, [&](){ uncheck(); });

    const QIcon loginIcon = QIcon::fromTheme("system-lock-screen");
    loginAct = new QAction(loginIcon, "&Login", self);
    loginAct->setStatusTip("Login to MinIO");
    self->connect(loginAct, &QAction::triggered, [&](){ login(); });

    const QIcon browseIcon = QIcon::fromTheme("applications-internet");
    browseAct = new QAction(browseIcon, "&Browse", self);
    browseAct->setStatusTip("Open a object browser");
    self->connect(browseAct, &QAction::triggered, [&](){ browse(); });

    const QIcon watchIcon = QIcon::fromTheme("system-search");
    watchAct = new QAction(watchIcon, "&Watch", self);
    watchAct->setStatusTip("Watch the directory");
    self->connect(watchAct, &QAction::triggered, [&](){ watcherWidget->show(); });
}

void MainWindow::Impl::createMenus()
{
    QMenu* menu = new QMenu(self);

    const QIcon systemIcon = QIcon::fromTheme("system-search");
    trayIcon = new QSystemTrayIcon(systemIcon);
    menu->addAction(watchAct);
    trayIcon->setContextMenu(menu);

    // trayIcon->show();
}

void MainWindow::Impl::createToolBars()
{
    bucketCombo = new QComboBox;
    bucketCombo->setStatusTip("Select the bucket");
    bucketCombo->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Preferred);
    self->connect(bucketCombo, &QComboBox::currentTextChanged, [&](QString text){ listItems(); });

    QToolBar* bucketToolBar = self->addToolBar("Bucket");
    bucketToolBar->addWidget(bucketCombo);
    bucketToolBar->addAction(listBucketAct);
    bucketToolBar->addAction(createBucketAct);
    bucketToolBar->addAction(delBucketAct);
    bucketToolBar->setObjectName("BucketToolBar");

    QToolBar* credentialToolBar = new QToolBar("Credential");
    credentialToolBar->addAction(loginAct);
    credentialToolBar->addAction(browseAct);
    credentialToolBar->setObjectName("CredentialToolBar");
    self->menuBar()->setCornerWidget(credentialToolBar);

    typeCombo = new QComboBox;
    typeCombo->setStatusTip("Select the content type");
    typeCombo->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Preferred);
    self->connect(typeCombo, QOverload<int>::of(&QComboBox::currentIndexChanged),
        [=](int index){ filterObjects(); });

    QToolBar* objectToolBar = self->addToolBar("Object");
    objectToolBar->addWidget(typeCombo);
    objectToolBar->addAction(listObjectAct);
    objectToolBar->addAction(putObjectAct);
    objectToolBar->addAction(delObjectAct);
    objectToolBar->addAction(headObjectAct);
    objectToolBar->addAction(getObjectAct);
    self->addToolBar(Qt::BottomToolBarArea, objectToolBar);
    self->setObjectName("ObjectToolBar");
}

void MainWindow::Impl::writeSettings()
{
    QSettings settings("MyCompany", "MinIOClient");
    settings.setValue("access_key", accessKeyName);
    settings.setValue("secret_key", secretKeyName);
    settings.setValue("endpoint", endpointName);
}

void MainWindow::Impl::readSettings()
{
    QSettings settings("MyCompany", "MinIOClient");
    accessKeyName = settings.value("access_key").toString();
    secretKeyName = settings.value("secret_key").toString();
    endpointName = settings.value("endpoint").toString();
}

void MainWindow::closeEvent(QCloseEvent* event)
{
    impl->writeSettings();
    QMainWindow::closeEvent(event);
}

CreateBucketDialog::CreateBucketDialog(QWidget* parent)
    : QDialog(parent)
{
    bucketLine = new QLineEdit;

    QFormLayout* layout = new QFormLayout;
    layout->addRow("Bucket", bucketLine);

    buttonBox = new QDialogButtonBox(QDialogButtonBox::Ok
                                     | QDialogButtonBox::Cancel);

    connect(buttonBox, &QDialogButtonBox::accepted, [&](){ accept(); });
    connect(buttonBox, &QDialogButtonBox::rejected, [&](){ reject(); }); 

    QVBoxLayout* mainLayout = new QVBoxLayout;
    mainLayout->addLayout(layout);
    mainLayout->addWidget(buttonBox);

    setLayout(mainLayout);
    setWindowTitle("Create a bucket");
}

TaggingDialog::TaggingDialog(QWidget* parent)
    : QDialog(parent)
{
    createMenu();

    const QStringList list = { "when", "where", "who", "what", "why", "how" };
    QGridLayout* gridLayout = new QGridLayout;
    for(int i = 0; i < 10; ++i) {
        QLineEdit* line = new QLineEdit;
        keyLines[i] = line;
        line->setPlaceholderText("Free key");
        if(i < list.size()) {
            line->setText(list.at(i));
            line->setEnabled(false);
        }

        QLineEdit* line2 = new QLineEdit;
        valueLines[i] = line2;

        const QStringList list = { QString("Key %1").arg(i), QString("Value %1").arg(i) };
        gridLayout->addWidget(new QLabel(list.at(0)), i, 0);
        gridLayout->addWidget(line, i, 1);
        gridLayout->addWidget(new QLabel(list.at(1)), i, 2);
        gridLayout->addWidget(line2, i, 3);
    }

    const QIcon calendarIcon = QIcon::fromTheme("x-office-calendar");
    QPushButton* calendarButton = new QPushButton(calendarIcon, "");
    connect(calendarButton, &QPushButton::clicked, [&](){ on_calendarButton_clicked(); });
    gridLayout->addWidget(calendarButton, 0,4);

    pathLine = new QLineEdit;

    QFormLayout* formLayout = new QFormLayout;
    formLayout->addRow("Path", pathLine);

    QPushButton* resetButton = new QPushButton("&Reset");
    connect(resetButton, &QPushButton::clicked, [&](){ on_resetButton_clicked(); });

    const QIcon putFileIcon = QIcon::fromTheme("document-new");
    QPushButton* putFileButton = new QPushButton(putFileIcon, "&Put");
    connect(putFileButton, &QPushButton::clicked, [&](){ on_putFileButton_clicked(); });

    const QIcon putDirIcon = QIcon::fromTheme("folder-new");
    QPushButton* putDirButton = new QPushButton(putDirIcon, "&Put");
    connect(putDirButton, &QPushButton::clicked, [&](){ on_putDirButton_clicked(); });

    buttonBox = new QDialogButtonBox(QDialogButtonBox::Ok
                                     | QDialogButtonBox::Cancel);

    connect(buttonBox, &QDialogButtonBox::accepted, [&](){ accept(); });
    connect(buttonBox, &QDialogButtonBox::rejected, [&](){ reject(); });    

    buttonBox->addButton(resetButton, QDialogButtonBox::ResetRole);
    buttonBox->addButton(putFileButton, QDialogButtonBox::ActionRole);
    buttonBox->addButton(putDirButton, QDialogButtonBox::ActionRole);

    QVBoxLayout* mainLayout = new QVBoxLayout;
    mainLayout->setMenuBar(menuBar);
    mainLayout->addLayout(formLayout);
    mainLayout->addLayout(gridLayout);
    mainLayout->addWidget(buttonBox);

    setLayout(mainLayout);
    setWindowTitle("Put objects");
}

void TaggingDialog::open()
{
    static QString dir = "/home";
    QString fileName = QFileDialog::getOpenFileName(this, "Open File",
        dir,
        "JSON Files (*.json);;All Files (*)");

    if(fileName.isEmpty()) {
        return;
    } else {
        QFileInfo info(fileName);
        dir = info.absolutePath();
        loadFile(fileName);
    }
}

void TaggingDialog::save()
{
    static QString dir = "/home";
    QString fileName = QFileDialog::getSaveFileName(this, "Save File",
        dir,
        "JSON Files (*.json);;All Files (*)");

    if(fileName.isEmpty()) {
        return;
    } else {
        QFileInfo info(fileName);
        dir = info.absolutePath();
        saveFile(fileName);
    }
}

void TaggingDialog::putObject(const QString& fileName)
{
    if(!credentials.ready()) {
        return;
    }

    if(!bucketName.isEmpty()) {
        BucketItem* bucketItem = new BucketItem(bucketName.toStdString());
        bucketItem->createCredentials(credentials.accessKeyName.toStdString(), credentials.secretKeyName.toStdString());

        QDir dir2(dirName);
        QString s = dir2.relativeFilePath(fileName);

        QFileInfo info(fileName);
        QString path = pathLine->text();
        QString objectKeyName;
        if(!path.isEmpty()) {
            objectKeyName = QString("%1/").arg(path);
        }
        objectKeyName += s;
        bucketItem->putObject(objectKeyName.toStdString(), fileName.toStdString(), *credentials.clientConfig);

        // tagging
        for(int i = 0; i < 10; ++i) {
            QString key = keyLines[i]->text();
            QString value = valueLines[i]->text();
            if(!key.isEmpty() && !value.isEmpty()) {
                bucketItem->putObjectTagging(objectKeyName.toStdString(), key.toStdString(), value.toStdString(), *credentials.clientConfig);
            }
        }
    }
}

void TaggingDialog::putObjects(const QString& fileName)
{
    QDir dir(fileName);
    QFileInfoList list = dir.entryInfoList();

    QProgressDialog progress("Putting objects...", "Cancel", 0, list.size(), this);
    progress.setWindowModality(Qt::WindowModal);

    for(int i = 0; i < list.size(); ++i) {
        progress.setValue(i);
        if(progress.wasCanceled()) {
            break;
        }
        QFileInfo fileInfo = list.at(i);
        if(!fileInfo.isDir()) {
            QString filePath = fileInfo.filePath();
            putObject(filePath);
        } else {
            QString fileName = fileInfo.fileName();
            if(fileName != "." && fileName != "..") {
                putObjects(fileInfo.filePath());
            }
        }
    }
    progress.setValue(list.size());
}

void TaggingDialog::on_resetButton_clicked()
{
    pathLine->clear();

    for(int i = 0; i < 10; ++i) {
        QLineEdit* line = keyLines[i];
        if(line->isEnabled()) {
            line->clear();
        }
        valueLines[i]->clear();
    }
}

void TaggingDialog::on_putFileButton_clicked()
{
    if(!completed()) {
        return;
    }

    static QString dir = "/home";
    QStringList files = QFileDialog::getOpenFileNames(
        this,
        "Select one or more files to open",
        dir,
        "All Files (*)");

    if(files.size() == 0) {
        return;
    } else {
        QFileInfo info(files.at(0));
        dir = info.absolutePath();

        QProgressDialog progress("Putting objects...", "Cancel", 0, files.size(), this);
        progress.setWindowModality(Qt::WindowModal);

        for(int i = 0; i < files.size(); ++i) {
            progress.setValue(i);
            if(progress.wasCanceled()) {
                break;
            }
            QString fileName = files.at(i);
            QFileInfo fileInfo(fileName);
            dirName = fileInfo.absolutePath();
            putObject(fileName);
        }
        progress.setValue(files.size());
    }
}

void TaggingDialog::on_putDirButton_clicked()
{
    if(!completed()) {
        return;
    }

    static QString dir2 = "/home";
    QString dir = QFileDialog::getExistingDirectory(this, "Open Directory",
        dir2,
        QFileDialog::ShowDirsOnly
        | QFileDialog::DontResolveSymlinks);

    if(dir.isEmpty()) {
        return;
    } else {
        dir2 = dir;
        dirName = dir;
        putObjects(dirName);
    }
}

void TaggingDialog::on_calendarButton_clicked()
{
    CalendarDialog dialog(this);

    if(dialog.exec()) {
        QDate date = dialog.selectedDate();
        valueLines[0]->setText(date.toString("yyyyMMdd"));
    }
}

bool TaggingDialog::completed() const
{
    bool is_completed = true;
    QList<int> maskList;
    for(int i = 0; i < 10; ++i) {
        if(!keyLines[i]->isEnabled()) {
            maskList << i;
            if(valueLines[i]->text().isEmpty()) {
                is_completed = false;
            }
        }
    }

    if(!is_completed) {
        QString text = "Enter Value";
        for(int i = 0; i < maskList.size(); ++i) {
            text += QString(" %1").arg(maskList.at(i));
        }

        QMessageBox msgBox;
        msgBox.setText(text);
        msgBox.setStandardButtons(QMessageBox::Ok);
        msgBox.setDefaultButton(QMessageBox::Ok);
        int ret = msgBox.exec();

        switch(ret) {
            case QMessageBox::Ok:
                // Ok was clicked
                break;
            default:
                // should never be reached
                break;
        }
    }

    return is_completed;
}

void TaggingDialog::read(const QJsonObject& json)
{
    pathLine->setText(get(json, "path", ""));

    if(json.contains("key") && json["key"].isArray()) {
        QJsonArray keyArray = json["key"].toArray();
        for(int i = 0; i < keyArray.size(); ++i) {
            keyLines[i]->setText(keyArray[i].toString());
        }
    }

    if(json.contains("value") && json["value"].isArray()) {
        const QJsonArray valueArray = json["value"].toArray();
        for(int i = 0; i < valueArray.size(); ++i) {
            valueLines[i]->setText(valueArray[i].toString());
        }
    }
}

void TaggingDialog::write(QJsonObject& json)
{
    json["path"] = pathLine->text();

    QJsonArray keyArray;
    QJsonArray valueArray;
    for(int i = 0; i < 10; ++i) {
        keyArray.append(keyLines[i]->text());
        valueArray.append(valueLines[i]->text());
    }
    json["key"] = keyArray;
    json["value"] = valueArray;
}

void TaggingDialog::createMenu()
{
    menuBar = new QMenuBar;

    fileMenu = new QMenu("&File", this);
    openAct = fileMenu->addAction("&Open...");
    saveAct = fileMenu->addAction("&Save");
    exitAct = fileMenu->addAction("E&xit");
    menuBar->addMenu(fileMenu);

    connect(openAct, &QAction::triggered, [&](){ open(); });
    connect(saveAct, &QAction::triggered, [&](){ save(); });
    connect(exitAct, &QAction::triggered, [&](){ accept(); });
}

HeadDialog::HeadDialog(QWidget* parent)
    : QDialog(parent)
{
    beginLine = new QLineEdit;

    endLine = new QLineEdit;
    
    const QIcon beginIcon = QIcon::fromTheme("x-office-calendar");
    QPushButton* beginButton = new QPushButton(beginIcon, "");
    connect(beginButton, &QPushButton::clicked, [&](){ on_beginButton_clicked(); });

    const QIcon endIcon = QIcon::fromTheme("x-office-calendar");
    QPushButton* endButton = new QPushButton(endIcon, "");
    connect(endButton, &QPushButton::clicked, [&](){ on_endButton_clicked(); });

    typeCombo = new QComboBox;
    typeCombo->addItems(QStringList() << "AND" << "OR");

    QHBoxLayout* layout = new QHBoxLayout;
    layout->addWidget(beginLine);
    layout->addWidget(beginButton);
    layout->addWidget(new QLabel("~"));
    layout->addWidget(endLine);
    layout->addWidget(endButton);
    layout->addWidget(typeCombo);

    QGridLayout* gridLayout = new QGridLayout;
    for(int i = 0; i < 10; ++i) {
        QComboBox* combo = new QComboBox;
        keyCombo[i] = combo;
        connect(combo, QOverload<int>::of(&QComboBox::currentIndexChanged),
            [=](int index){ on_keyCombo_currentIndexChanged(i); });

        QComboBox* combo2 = new QComboBox;
        valueCombo[i] = combo2;
        
        const QStringList list = { QString("Key %1").arg(i), QString("Value %1").arg(i) };
        gridLayout->addWidget(new QLabel(list.at(0)), i, 0);
        gridLayout->addWidget(combo, i, 1);
        gridLayout->addWidget(new QLabel(list.at(1)), i, 2);
        gridLayout->addWidget(combo2, i, 3);
    }

    QPushButton* resetButton = new QPushButton("&Reset");
    connect(resetButton, &QPushButton::clicked, [&](){ on_resetButton_clicked(); });

    buttonBox = new QDialogButtonBox(QDialogButtonBox::Ok
                                     | QDialogButtonBox::Cancel);

    connect(buttonBox, &QDialogButtonBox::accepted, [&](){ head(); });
    connect(buttonBox, &QDialogButtonBox::rejected, [&](){ reject(); });

    buttonBox->addButton(resetButton, QDialogButtonBox::ResetRole);

    QVBoxLayout* mainLayout = new QVBoxLayout;
    mainLayout->addLayout(layout);
    mainLayout->addLayout(gridLayout);
    mainLayout->addWidget(buttonBox);

    setLayout(mainLayout);
    setWindowTitle("Head objects");
}

void HeadDialog::update()
{
    for(int i = 0; i < 10; ++i) {
        keyCombo[i]->addItem("");
        keyCombo[i]->addItems(taggingKeyList);
    }
}

void HeadDialog::head()
{
    if(!credentials.ready()) {
        return;
    }

    QStringList list[10];
    headKeyList.clear();
    bool is_selected = false;

    Aws::Vector<Aws::S3::Model::Tag> tags;
    for(int i = 0; i < 10; ++i) {
        QString key = keyCombo[i]->currentText();
        QString value = valueCombo[i]->currentText();
        Aws::S3::Model::Tag tag;
        tag.SetKey(key.toStdString());
        tag.SetValue(value.toStdString());
        tags.push_back(tag);
        if(!key.isEmpty()) {
            is_selected = true;
        }
    }

    if(!bucketName.isEmpty()) {
        BucketItem* bucketItem = new BucketItem(bucketName.toStdString());
        bucketItem->createCredentials(credentials.accessKeyName.toStdString(), credentials.secretKeyName.toStdString());

        QStringList keyList;
        QString beginText = beginLine->text();
        QString endText = endLine->text();
        if(!beginText.isEmpty() && !endText.isEmpty()) {
            int begin = beginText.toInt();
            int end = endText.toInt();

            QProgressDialog progress("Head objects...", "Cancel", 0, objectKeyList.size(), this);
            progress.setWindowModality(Qt::WindowModal);

            for(int i = 0; i < objectKeyList.size(); ++i) {
                progress.setValue(i);
                if(progress.wasCanceled()) {
                    break;
                }
                QString objectKeyName = objectKeyList.at(i);
                Aws::Vector<Aws::S3::Model::Tag> allTags = bucketItem->getObjectsTagging(objectKeyName.toStdString(), *credentials.clientConfig);
                for(int j = 0; j < allTags.size(); ++j) {
                    Aws::S3::Model::Tag tag = allTags[j];
                    QString key = tag.GetKey().c_str();
                    QString value = tag.GetValue().c_str();
                    if(key == "when") {
                        int date = atoi(value.toStdString().c_str());
                        if(begin <= date && date <= end) {
                            keyList << objectKeyName;
                        }
                    }
                }
            }
            progress.setValue(objectKeyList.size());
        } else {
            keyList = objectKeyList;
        }

        if(is_selected) {
            QProgressDialog progress("Head objects...", "Cancel", 0, keyList.size(), this);
            progress.setWindowModality(Qt::WindowModal);

            for(int i = 0; i < keyList.size(); ++i) {
                progress.setValue(i);
                if(progress.wasCanceled()) {
                    break;
                }
                QString objectKeyName = keyList.at(i);
                Aws::Vector<Aws::S3::Model::Tag> allTags = bucketItem->getObjectsTagging(objectKeyName.toStdString(), *credentials.clientConfig);

                for(int j = 0; j < tags.size(); ++j) {
                    Aws::S3::Model::Tag tag0 = tags[j];
                    for(int k = 0; k < allTags.size(); ++k) {
                        Aws::S3::Model::Tag tag1 = allTags[k];
                        if(tag0.GetKey() == tag1.GetKey()
                            && tag0.GetValue() == tag1.GetValue()) {
                            list[j] << objectKeyName;
                        }
                    }
                }
            }
            progress.setValue(keyList.size());

            for(int i = 0; i < 10; ++i) {
                headKeyList << list[i];
            }
            headKeyList.removeDuplicates();
        } else {
            headKeyList = keyList;
        }
    }

    QString type = typeCombo->currentText();
    if(type == "AND") {
        QStringList popList;
        for(int i = 0; i < headKeyList.size(); ++i) {
            QString key = headKeyList.at(i);
            for(int j = 0; j < 10; ++j) {
                if(list[j].size() > 0) {
                    if(!list[j].contains(key)) {
                        popList << key;
                    }
                }
            }
        }

        for(int i = 0; i < popList.size(); ++i) {
            headKeyList.removeAll(popList.at(i));
        }
    } else if(type == "OR") {

    }

    // make message
    message = QString("%1 Heading by").arg(typeCombo->currentText());
    for(int i = 0; i < 10; ++i) {
        QString key = keyCombo[i]->currentText();
        QString value = valueCombo[i]->currentText();
        if(!value.isEmpty()) {
            message += QString(" %1=%2,").arg(key).arg(value);
        }
    }

    QString beginText = beginLine->text();
    QString endText = endLine->text();
    if(!beginText.isEmpty() && !endText.isEmpty()) {
        is_selected = true;
        message += QString(" (%1~%2)").arg(beginText).arg(endText);
    } else {
        if(message.contains(",")) {
            message.chop(1);
        }
    }

    accept();
}

void HeadDialog::on_resetButton_clicked()
{
    typeCombo->setCurrentIndex(0);
    beginLine->clear();
    endLine->clear();
    for(int i = 0; i < 10; ++i) {
        keyCombo[i]->setCurrentIndex(0);
        valueCombo[i]->setCurrentIndex(0);
    }
}

void HeadDialog::on_keyCombo_currentIndexChanged(int arg1)
{
    QComboBox* combo = valueCombo[arg1];
    combo->clear();
    QString key = keyCombo[arg1]->currentText();
    QStringList list = valueMap.value(key);
    combo->addItem("");
    combo->addItems(list);
}

void HeadDialog::on_beginButton_clicked()
{
    CalendarDialog dialog(this);
    dialog.setMaximumDate(endDate);

    if(dialog.exec()) {
        beginDate = dialog.selectedDate();
        beginLine->setText(beginDate.toString("yyyyMMdd"));
        if(endLine->text().isEmpty()) {
            endDate = beginDate;
            endLine->setText(endDate.toString("yyyyMMdd"));
        }
    }
}

void HeadDialog::on_endButton_clicked()
{
    CalendarDialog dialog(this);
    dialog.setMinimumDate(beginDate);

    if(dialog.exec()) {
        endDate = dialog.selectedDate();
        endLine->setText(endDate.toString("yyyyMMdd"));
        if(beginLine->text().isEmpty()) {
            beginDate = endDate;
            beginLine->setText(beginDate.toString("yyyyMMdd"));
        }
    }
}

InfoDialog::InfoDialog(QWidget* parent)
    : QDialog(parent)
{
    nameLabel = new QLabel;

    sizeLabel = new QLabel;

    lastModifiedLabel = new QLabel;
    
    eTagLabel = new QLabel;

    tagTree = new QTreeWidget;
    tagTree->setHeaderLabels(QStringList() << "Key" << "Value");
    tagTree->setContextMenuPolicy(Qt::CustomContextMenu);

    QFormLayout* layout = new QFormLayout;
    layout->addRow("Name", nameLabel);
    layout->addRow("Size", sizeLabel);
    layout->addRow("Last Modified", lastModifiedLabel);
    layout->addRow("ETAG", eTagLabel);

    QVBoxLayout* mainLayout = new QVBoxLayout;
    mainLayout->addLayout(layout);
    mainLayout->addWidget(tagTree);

    setLayout(mainLayout);
    setWindowTitle("Object info");
}

void InfoDialog::update()
{
    if(!credentials.ready()) {
        return;
    }

    tagTree->clear();
    nameLabel->setText(objectKeyName);

    if(!bucketName.isEmpty()) {
        BucketItem* bucketItem = new BucketItem(bucketName.toStdString());
        bucketItem->createCredentials(credentials.accessKeyName.toStdString(), credentials.secretKeyName.toStdString());

        Aws::Vector<Aws::S3::Model::Tag> tags = bucketItem->getObjectsTagging(objectKeyName.toStdString(), *credentials.clientConfig);
        for(int i = 0; i < tags.size(); ++i) {
            Aws::S3::Model::Tag tag = tags[i];
            QTreeWidgetItem* item = new QTreeWidgetItem(tagTree);
            item->setText(0, tag.GetKey().c_str());
            item->setText(1, tag.GetValue().c_str());
        }

        long long contentLength = bucketItem->getContentLength(objectKeyName.toStdString(), *credentials.clientConfig);
        double size = (double)contentLength / (1024.0 * 1024.0);
        sizeLabel->setText(QString("%1 MiB").arg(size));

        Aws::Utils::DateTime dateTime = bucketItem->getLastModified(objectKeyName.toStdString(), *credentials.clientConfig);
        int year = dateTime.GetYear();
        int month = (int)dateTime.GetMonth() + 1;
        int day = dateTime.GetDay();
        QDate date(year, month, day);
        lastModifiedLabel->setText(date.toString("yyyyMMdd"));

        QString eTag = bucketItem->getETag(objectKeyName.toStdString(), *credentials.clientConfig).c_str();
        eTagLabel->setText(eTag);
    }
}

EditDialog::EditDialog(QWidget* parent)
    : QDialog(parent)
{
    QGridLayout* layout = new QGridLayout;
    for(int i = 0; i < 10; ++i) {
        QLineEdit* line = new QLineEdit;
        keyLines[i] = line;
        
        QLineEdit* line2 = new QLineEdit;
        valueLines[i] = line2;

        const QStringList list = { QString("Key %1").arg(i), QString("Value ").arg(i) };
        layout->addWidget(new QLabel(list.at(0)), i, 0);
        layout->addWidget(line, i, 1);
        layout->addWidget(new QLabel(list.at(1)), i, 2);
        layout->addWidget(line2, i, 3);
    }

    QPushButton* resetButton = new QPushButton("&Reset");
    connect(resetButton, &QPushButton::clicked, [&](){ on_resetButton_clicked(); });

    buttonBox = new QDialogButtonBox(QDialogButtonBox::Ok
                                     | QDialogButtonBox::Cancel);

    connect(buttonBox, &QDialogButtonBox::accepted, [&](){ overwrite(); });
    connect(buttonBox, &QDialogButtonBox::rejected, [&](){ reject(); });

    buttonBox->addButton(resetButton, QDialogButtonBox::ResetRole);

    QVBoxLayout* mainLayout = new QVBoxLayout;
    mainLayout->addLayout(layout);
    mainLayout->addWidget(buttonBox);

    setLayout(mainLayout);
    setWindowTitle("Edit object tagging");
}

void EditDialog::update()
{
    if(!credentials.ready()) {
        return;
    }

    if(!bucketName.isEmpty()) {
        BucketItem* bucketItem = new BucketItem(bucketName.toStdString());
        bucketItem->createCredentials(credentials.accessKeyName.toStdString(), credentials.secretKeyName.toStdString());

        Aws::Vector<Aws::S3::Model::Tag> tags = bucketItem->getObjectsTagging(objectKeyName.toStdString(), *credentials.clientConfig);
        for(int i = 0; i < tags.size(); ++i) {
            Aws::S3::Model::Tag tag = tags[i];
            keyLines[i]->setText(tag.GetKey().c_str());
            valueLines[i]->setText(tag.GetValue().c_str());
        }
    }
}

void EditDialog::overwrite()
{
    QString text = "Do you want to overwrite object tagging?";

    QMessageBox msgBox;
    msgBox.setText(text);
    msgBox.setStandardButtons(QMessageBox::Ok | QMessageBox::Cancel);
    msgBox.setDefaultButton(QMessageBox::Ok);
    int ret = msgBox.exec();

    switch(ret) {
    case QMessageBox::Ok:
        // Ok was clicked
        tagging();
        accept();
        break;
    case QMessageBox::Cancel:
        // Cancel was clicked
        reject();
        break;
    default:
        // should never be reached
        break;
    }
}

void EditDialog::tagging()
{
    if(!credentials.ready()) {
        return;
    }

    if(!bucketName.isEmpty()) {
        BucketItem* bucketItem = new BucketItem(bucketName.toStdString());
        bucketItem->createCredentials(credentials.accessKeyName.toStdString(), credentials.secretKeyName.toStdString());

        bucketItem->deleteObjectTagging(objectKeyName.toStdString(), *credentials.clientConfig);
        for(int i = 0; i < 10; ++i) {
            QString key = keyLines[i]->text();
            QString value = valueLines[i]->text();
            if(!key.isEmpty() && !value.isEmpty()) {
                bucketItem->putObjectTagging(objectKeyName.toStdString(), key.toStdString(), value.toStdString(), *credentials.clientConfig);
            }
        }
    }
}

void EditDialog::on_resetButton_clicked()
{
    for(int i = 0; i < 10; ++i) {
        keyLines[i]->clear();
        valueLines[i]->clear();
    }
}

CredentialDialog::CredentialDialog(QWidget* parent)
    : QDialog(parent)
{
    createMenu();

    accessKeyIdLine = new QLineEdit;

    secretKeyLine = new QLineEdit;
    secretKeyLine->setEchoMode(QLineEdit::Password);

    endpointLine = new QLineEdit;
    endpointLine->setPlaceholderText("http://xxxx.xxx.xxx.xxx:port");

    QFormLayout* layout = new QFormLayout;
    layout->addRow("User", accessKeyIdLine);
    layout->addRow("Password", secretKeyLine);
    layout->addRow("Endpoint", endpointLine);

    buttonBox = new QDialogButtonBox(QDialogButtonBox::Ok
                                     | QDialogButtonBox::Cancel);

    connect(buttonBox, &QDialogButtonBox::accepted, [&](){ accept(); });
    connect(buttonBox, &QDialogButtonBox::rejected, [&](){ reject(); });

    QVBoxLayout* mainLayout = new QVBoxLayout;
    mainLayout->setMenuBar(menuBar);
    mainLayout->addLayout(layout);
    mainLayout->addWidget(buttonBox);

    setLayout(mainLayout);
    setWindowTitle("Login");
}

void CredentialDialog::open()
{
    static QString dir = "/home";
    QString fileName = QFileDialog::getOpenFileName(this, "Open File",
        dir,
        "JSON Files (*.json);;All Files (*)");

    if(fileName.isEmpty()) {
        return;
    } else {
        QFileInfo info(fileName);
        dir = info.absolutePath();
        loadFile(fileName);
    }
}

void CredentialDialog::save()
{
    static QString dir = "/home";
    QString fileName = QFileDialog::getSaveFileName(this, "Save File",
        dir,
        "JSON Files (*.json);;All Files (*)");

    if(fileName.isEmpty()) {
        return;
    } else {
        QFileInfo info(fileName);
        dir = info.absolutePath();
        saveFile(fileName);
    }
}

void CredentialDialog::createMenu()
{
    menuBar = new QMenuBar;

    fileMenu = new QMenu("&File", this);
    openAct = fileMenu->addAction("&Open...");
    saveAct = fileMenu->addAction("&Save");
    exitAct = fileMenu->addAction("E&xit");
    menuBar->addMenu(fileMenu);

    connect(openAct, &QAction::triggered, [&](){ open(); });
    connect(saveAct, &QAction::triggered, [&](){ save(); });
    connect(exitAct, &QAction::triggered, [&](){ accept(); });
}

void CredentialDialog::read(const QJsonObject& json)
{
    accessKeyIdLine->setText(get(json, "access_key", ""));
    secretKeyLine->setText(get(json, "secret_key", ""));
    endpointLine->setText(get(json, "endpoint", ""));
}

void CredentialDialog::write(QJsonObject& json)
{
    json["access_key"] = accessKeyIdLine->text();
    json["secret_key"] = secretKeyLine->text();
    json["endpoint"] = endpointLine->text();
}

CalendarDialog::CalendarDialog(QWidget* parent)
    : QDialog(parent)
{

    calendar = new QCalendarWidget;

    buttonBox = new QDialogButtonBox(QDialogButtonBox::Ok
                                     | QDialogButtonBox::Cancel);

    connect(buttonBox, &QDialogButtonBox::accepted, [&](){ accept(); });
    connect(buttonBox, &QDialogButtonBox::rejected, [&](){ reject(); });

    QVBoxLayout* mainLayout = new QVBoxLayout;
    mainLayout->addWidget(calendar);
    mainLayout->addWidget(buttonBox);

    setLayout(mainLayout);
    setWindowTitle("Select a date");
}

WatcherWidget::WatcherWidget(QWidget* parent)
    : QWidget(parent)
{
    createActions();

    setWindowTitle("Directory Watcher");

    fileList = new QListWidget;
    fileList->setContextMenuPolicy(Qt::CustomContextMenu);
    connect(fileList, &QListWidget::customContextMenuRequested,
        [&](QPoint pos){ on_fileList_customContextMenuRequested(pos); });

    systemWatcher = new QFileSystemWatcher;
    connect(systemWatcher, &QFileSystemWatcher::directoryChanged,
        [&](QString path){ on_systemWatcher_directoryChanged(path); });

    QVBoxLayout* layout = new QVBoxLayout;
    layout->addWidget(fileList);
    setLayout(layout);
}

void WatcherWidget::add()
{
    static QString dir2 = "/home";
    QString dir = QFileDialog::getExistingDirectory(this, "Open Directory",
        dir2,
        QFileDialog::ShowDirsOnly
        | QFileDialog::DontResolveSymlinks);

    if(dir.isEmpty()) {
        return;
    } else {
        dir2 = dir;
        systemWatcher->addPath(dir);
        on_systemWatcher_directoryChanged(dir);
    }
}

void WatcherWidget::on_systemWatcher_directoryChanged(const QString& path)
{
    while(fileList->count() > 0) {
        fileList->takeItem(0);
    }

    QDir dir(path);
    QFileInfoList list = dir.entryInfoList();
    for(int i = 0; i < list.size(); ++i) {
        QFileInfo fileInfo = list.at(i);
        if(!fileInfo.isDir()) {
            fileList->addItem(fileInfo.filePath());
        }
    }
}

void WatcherWidget::on_fileList_customContextMenuRequested(const QPoint& pos)
{
    QMenu menu(this);
    menu.addAction(addAct);
    menu.exec(fileList->mapToGlobal(pos));
}

void WatcherWidget::createActions()
{
    addAct = new QAction("&Add", this);
    addAct->setStatusTip("Add the directory");
    connect(addAct, &QAction::triggered, [&](){ add(); });
}

}
