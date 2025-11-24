#include "mainwindow.h"
#include "ui_mainwindow.h"

#include <QFile>
#include <QFileInfo>
#include <QFileDialog>
#include <QDateTime>
#include <QList>
#include <QString>
#include <QStringList>
#include <QTreeWidgetItem>
#include <QSize>
#include <QRegularExpression>
#include <QMessageBox>
#include <QTimer>
#include <QCloseEvent>
#include <QCheckBox>
#include <QMovie>
#include <QDesktopServices>
#include <QShortcut>
#include <QDebug>
#include <controller.h>
#include <QCompleter>
#include <QStandardPaths>
#include <QTextEdit>
#include <QClipboard>
#include <QRandomGenerator>
#include <QToolTip>


MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow)
{
    ui->setupUi(this);
}


MainWindow::~MainWindow()
{
    delete ui;
}


void MainWindow::bootStrap()
{
    setLabelsVisible(QStringList() << "progressBar_status" << "label_progress", false);              // initial widget states setting
    setWidgetsDisabled(QStringList() << "pushButton_clearQueue" << "pushButton_copyScreenshots" << "pushButton_prepare", true);
    setDirStatusLabelsVisible(false);

    ui->label_progress->setMovie(gifLoader);
    gifLoader->start();

    QSizePolicy spRetain = ui->label_infoScreenshots->sizePolicy();     // hack to prevent layout size change on a widget visibility changing events
    spRetain.setRetainSizeWhenHidden(true);
    ui->label_infoScreenshots->setSizePolicy(spRetain);
    ui->progressBar_status->setSizePolicy(spRetain);

    emit sendButtonList(QList<QPushButton*>() << ui->pushButton_clearQueue << ui->pushButton_copyScreenshots    // buttons should have specific padding
                        << ui->pushButton_addScreenshots << ui->pushButton_prepare);                            // ...in each OS

    userIDComboBox = ui->comboBox_userID;
    // QObject::connect(userIDComboBox, static_cast<void(QComboBox::*)(const QString &)>(&QComboBox::activated),
    //                  this, &MainWindow::reactToComboBoxActivation);

    connect(userIDComboBox, &QComboBox::textActivated,
            this, &MainWindow::reactToComboBoxActivation);

    QTreeWidgetDragAndDrop *treeWidget = ui->treeWidget_screenshotList;
    emit sendTreeWidgetPointer(treeWidget);

    connect(ui->comboBox_gameID, &QComboBox::textActivated,
            this, &MainWindow::onGameSelected);

    connect(ui->comboBox_gameID, &QComboBox::currentTextChanged,
            controller, &Controller::loadFirstScreenshotForGame);


    connect(controller, &Controller::sendPreviewImage,
            this, &MainWindow::showPreviewImage);

    connect(ui->comboBox_userID, &QComboBox::currentTextChanged,
            controller, &Controller::onUserIDSelected);

    connect(controller, &Controller::sendPreviewCount,
            this, [this](int current, int total){
                ui->horizontalScrollBarScreenshots->setMaximum(total - 1);
                ui->horizontalScrollBarScreenshots->setValue(current - 1);
            });

    connect(controller, &Controller::apiReachabilityChanged,
            this, &MainWindow::onApiReachabilityChanged);

    connect(controller, &Controller::updateStatusMessage,
            this, [this](const QString &msg){
                ui->statusBar->showMessage(msg, 5000);
            });


    // Autovervollständigung für Spielnamen aktivieren
    QCompleter *completer = new QCompleter(ui->comboBox_gameID->model(), this);
    completer->setCaseSensitivity(Qt::CaseInsensitive);    // Groß-/Kleinschreibung ignorieren
    completer->setFilterMode(Qt::MatchContains);           // auch Teiltreffer in der Mitte erlauben
    completer->setCompletionMode(QCompleter::PopupCompletion); // Vorschläge in Dropdown anzeigen
    ui->comboBox_gameID->setCompleter(completer);

    setFooter();
}


void MainWindow::showPreviewImage(QPixmap pixmap)
{
    if (pixmap.isNull()) {
        ui->pushButtonPreview->setText("No screenshot found");
        ui->pushButtonPreview->setIcon(QIcon()); // Icon zurücksetzen
        m_previewOriginal = QPixmap(); //zurücksetzen
        return;
    }

    m_previewOriginal = pixmap;

    // Aufnahmedatum anzeigen
    if (controller) {
        QString path = controller->getCurrentScreenshotPath();
        if (!path.isEmpty()) {
            QFileInfo info(path);
            //QString date = info.birthTime().toString("yyyy-MM-dd hh:mm:ss");
            QString date = info.birthTime().toString("d MMMM yyyy, HH:mm:ss");
            ui->label_previewDate->setText("Captured On: " + date);
        }
    }

    QPixmap scaled = pixmap.scaled(
        ui->pushButtonPreview->size(),
        Qt::KeepAspectRatio,
        Qt::SmoothTransformation
        );

    ui->pushButtonPreview->setIcon(QIcon(scaled));
    ui->pushButtonPreview->setIconSize(scaled.size());
    ui->pushButtonPreview->setText(""); //Text entfernen

    // 2. Durchlauf, damit die Skalierung des Bildes sofort passt
    QTimer::singleShot(0, this, [this](){
 if (m_previewOriginal.isNull())
            return;

        QSize finalSize = ui->pushButtonPreview->size();

        QPixmap scaledFinal = m_previewOriginal.scaled(
            finalSize,
            Qt::KeepAspectRatio,
            Qt::SmoothTransformation
            );
        ui->pushButtonPreview->setIcon(QIcon(scaledFinal));
        ui->pushButtonPreview->setIconSize(scaledFinal.size());
    });

}


void MainWindow::reactToComboBoxActivation(QString userID)
{
    emit sendNewlySelectedUserID(userID.remove(QRegularExpression(" <.+>$")));
}


void MainWindow::onGameSelected(QString gameID)
{
    emit sendNewlySelectedGameID(gameID.remove(QRegularExpression(" <.+>$")));
}


void MainWindow::addWidgetItemToScreenshotList(QTreeWidgetItem *item)
{
    ui->treeWidget_screenshotList->addTopLevelItem(item);
}


void MainWindow::resizeScreenshotListColumns()
{
    ui->treeWidget_screenshotList->resizeColumnToContents(0); // when all is added, resize columns for a better appearance
    ui->treeWidget_screenshotList->resizeColumnToContents(1);
}


void MainWindow::makeWideMessageBox(QMessageBox *msgBox, quint32 width) // hack to make wide message boxes
{
    QGridLayout* layout = (QGridLayout*)msgBox->layout();
    layout->addItem(new QSpacerItem(width, 0, QSizePolicy::Minimum, QSizePolicy::Expanding), layout->rowCount(), 0, 1, layout->columnCount());
}


void MainWindow::locateSteamDir(QString steamDir)
{
    QString steamDirLocated = QFileDialog::getExistingDirectory(this,
                                                                "Locate Steam directory",
                                                                steamDir,
                                                                QFileDialog::ShowDirsOnly | QFileDialog::ReadOnly);
    if ( !steamDirLocated.isEmpty() ) {

        steamDirLocated.remove(QRegularExpression("/userdata$"));
        emit sendUserDataPaths(steamDirLocated);
    }
}


void MainWindow::offerUpdate(QString version, QString link)
{
    QMessageBox msgBox(this);
    msgBox.setIcon(QMessageBox::Question);
    QString text = "New version available.";
    QString info = "SteaScreeLoaded version " + version + " is available online. Would you like to download it?";
    msgBox.setText(text);
    msgBox.setInformativeText(info);
    QPushButton *never = msgBox.addButton("Never", QMessageBox::RejectRole);
    msgBox.setStandardButtons(QMessageBox::Yes | QMessageBox::No);
    msgBox.setDefaultButton(QMessageBox::Yes);

    makeWideMessageBox(&msgBox, 400);
    int ret = msgBox.exec();

    if ( ret == QMessageBox::Yes )
        QDesktopServices::openUrl(QUrl(link));
    else if ( msgBox.clickedButton() == never )
        emit sendNeverOfferUpdate();
}


void MainWindow::setLabelsText(QStringList list, QString text)
{
    QStringListIterator i(list);
    while ( i.hasNext() ) {
        QString current = i.next();
        QLabel *label = this->findChild<QLabel*>(current);
        label->setText(text);
    }
}


void MainWindow::setWidgetsDisabled(QStringList list, bool disable)
{
    QStringListIterator i(list);
    while ( i.hasNext() ) {
        QString current = i.next();
        QWidget *widget = this->findChild<QWidget*>(current);
        widget->setDisabled(disable);
    }
}


void MainWindow::setLabelsVisible(QStringList list, bool visible)
{
    QStringListIterator i(list);
    while ( i.hasNext() ) {
        QString current = i.next();
        QWidget *widget = this->findChild<QWidget*>(current);
        widget->setVisible(visible);
    }
}


void MainWindow::setDirStatusLabelsVisible(bool visible) // info labels show/hide toggle
{
    QStringList labelInfoList = QStringList() << "label_infoScreenshots" << "label_infoScreenshotsCopied"
                                              << "label_infoDirectories" << "label_infoDirectoriesCopied";
    setLabelsVisible(labelInfoList, visible);
}


void MainWindow::setComboBoxesCleared(QStringList list)
{
    QStringListIterator i(list);
    while ( i.hasNext() ) {
        QString current = i.next();
        QComboBox *widget = this->findChild<QComboBox*>(current);
        widget->clear();
    }
}


void MainWindow::setLabelsCleared(QStringList list)
{
    QStringListIterator i(list);
    while ( i.hasNext() ) {
        QString current = i.next();
        QLabel *label = this->findChild<QLabel*>(current);
        label->clear();
    }
}


void MainWindow::setProgressBarLength(quint32 length)
{
    ui->progressBar_status->setMaximum(length);
}


void MainWindow::setProgressBarValue(quint32 value)
{
    ui->progressBar_status->setValue(value);
}


void MainWindow::moveWindow(QSize geometry, QPoint moveToPoint)
{
    resize(geometry);
    move(moveToPoint);
}


void MainWindow::insertIntoComboBox(QString name, QStringList items)
{
    QComboBox *comboBox = this->findChild<QComboBox*>(name);
    comboBox->insertItems(0, items);

    //Beim Start vorhandene IDs mit Namen aus Datei ergänzen:
    updateComboBoxNames();

    //prüfe aktuell gewählten Eintrag auf benutzerdefinierten Namen
    checkModifiedGameList(ui->comboBox_gameID->currentText());
}

void MainWindow::setIndexOfComboBox(QString name, QString text)
{
    QComboBox *comboBox = this->findChild<QComboBox*>(name);
    comboBox->setCurrentIndex(comboBox->findText(text, Qt::MatchStartsWith));
}

void MainWindow::setIndexOfComboBoxAPI(QString name, int index)
{
    QComboBox *comboBox = this->findChild<QComboBox*>(name);
    comboBox->setCurrentIndex(index);
}


void MainWindow::setLabelsOnMissingStuff(bool userDataMissing, QString vdfFilename)
{
    ui->label_status->setVisible(true);
    if ( userDataMissing )
        setStatusLabelText("Steam \"userdata\" directory is not found", warningColor);
    else
        setStatusLabelText("Steam \"userdata\" directory is found, but " + vdfFilename + " is missing", warningColor);

    ui->label_steamDirValue->setText("not found");
    ui->label_steamDirValue->setStyleSheet("color: gray;");
}


void MainWindow::returnScreenshotsSelected(QString lastSelectedScreenshotDir)
{
    QStringList screenshotsSelected = QFileDialog::getOpenFileNames(this,
                                                                    "Select one or several screenshots",
                                                                    lastSelectedScreenshotDir,
                                                                    "Images (*.jpg *.jpeg *.png *.bmp *.tif *.tiff)");
    emit sendScreenshotsSelected(screenshotsSelected);
}


void MainWindow::deleteCopiedWidgetItem(QString path)
{
    QFile file(path);
    QTreeWidgetItem *item = ui->treeWidget_screenshotList->findItems(QFileInfo(file).lastModified()
                                                                     .toString("yyyy/MM/dd hh:mm:ss"), Qt::MatchExactly, 1)[0];
    delete item;
}


void MainWindow::disableAllControls()
{
    setWidgetsDisabled(QStringList() << "pushButton_clearQueue" << "pushButton_addScreenshots" << "pushButton_copyScreenshots"
                       << "pushButton_locateSteamDir" << "comboBox_userID" << "comboBox_gameID" << "pushButton_prepare", true);
}


void MainWindow::setStatusLabelText(QString text, QString color)
{
    ui->label_status->setText(text);
    if (color.isEmpty())
        color = "black";
    ui->label_status->setStyleSheet("QLabel {color: " + color + "};");
}


void MainWindow::setJpegQualityValue(quint32 jpegQualityValue)
{
    ui->spinBox_jpegQuality->setValue(jpegQualityValue);
}


void MainWindow::prepareScreenshots(quint32 addedLines)
{
    // Prüfen, ob Steam läuft
    // Wenn Steam läuft oder nicht festgestellt werden kann, sofort warnen
    if (isSteamRunning()) {
        QMessageBox msg(this);
        msg.setIcon(QMessageBox::Warning);
        msg.setWindowTitle("Steam must be closed");
        msg.setText("Steam appears to be running.");
        msg.setInformativeText("This programme requires Steam to be fully closed before it can proceed. "
                               "If you are certain that no interfering Steam process is active, "
                               "you may continue at your own risk.");
        msg.setStandardButtons(QMessageBox::Abort | QMessageBox::Ignore);
        msg.setDefaultButton(QMessageBox::Abort);

        int ret = msg.exec();
        if (ret == QMessageBox::Abort) {
            return; // Abbrechen
        }
        // Bei Ignore fortsetzen (falls automatische Prüfung fehlerhaft ist)
    }

    if (addedLines > 0){

        // QMessageBox msgBox(this);
        // msgBox.setIcon(QMessageBox::Warning);

        // QString text = "Steam has to be quitted.";
        // QString info = "This programme only works when Steam has exited. "
        //                "If it is still running, it is safe to close Steam now. "
        //                 "<br><br>Is Steam closed now?";
        // msgBox.setText(text);
        // msgBox.setInformativeText(info);
        // msgBox.setStandardButtons(QMessageBox::Yes | QMessageBox::No);
        // msgBox.setDefaultButton(QMessageBox::Yes);

        // makeWideMessageBox(&msgBox, 500);

        // int ret = msgBox.exec();

        // if ( ret == QMessageBox::Yes ) {

            setDirStatusLabelsVisible(false);
            setLabelsText(QStringList() << "label_infoScreenshots" << "label_infoDirectories", "0");
            ui->pushButton_prepare->setDisabled(true);

            emit writeVDF();
            emit clearScreenshotPathsPool();
            emit clearState();

            setStatusLabelText("ready", "");

            QMessageBox msgBox(this);
            msgBox.setIcon(QMessageBox::Information);
            msgBox.setText("SteaScreeLoaded has updated the VDF-file.");
            msgBox.setInformativeText("You may now start Steam as usual and upload screenshots to the Steam Cloud.");
            msgBox.exec();
        // }

    } else {
        QMessageBox msgBox(this);
        msgBox.setIcon(QMessageBox::Warning);
        msgBox.setText("All screenshots from the upload queue are already in the screenshots.vdf file.");
        msgBox.setInformativeText("Nothing has been added. Please add new screenshots and try again.");
        msgBox.exec();

        setDirStatusLabelsVisible(false);
        ui->pushButton_prepare->setDisabled(true);
    }

    emit clearCopyingStatusLabels();
}


void MainWindow::on_pushButton_locateSteamDir_clicked()
{
    emit getSteamDir();
}


void MainWindow::on_pushButton_addScreenshots_clicked()
{
    emit pushButton_addScreenshots_clicked();
}


void MainWindow::on_pushButton_clearQueue_clicked()
{  
    ui->treeWidget_screenshotList->clear();

    setWidgetsDisabled(QStringList() << "pushButton_clearQueue" << "pushButton_copyScreenshots", true);

    setWidgetsDisabled(QStringList() << "label_userID" << "comboBox_gameID" << "label_gameID"
                       << "comboBox_userID" << "pushButton_locateSteamDir", false);

    emit clearScreenshotPathsPool();
}


void MainWindow::on_pushButton_copyScreenshots_clicked()
{
    QString selectedUserID = ui->comboBox_userID->currentText();
    QString selectedGameID = ui->comboBox_gameID->currentText();
    quint32 jpegQuality = ui->spinBox_jpegQuality->value();

    QRegularExpression re("^[0-9]+( <.+>)?$");

    if ( !selectedGameID.contains(re) )
        setStatusLabelText("invalid game ID, only numbers allowed", warningColor);
    else {  // valid game ID

        disableAllControls();
        ui->label_status->clear();
        setLabelsVisible(QStringList() << "label_status" << "label_progress" << "progressBar_status", true);
        setWidgetsDisabled( QStringList() << "pushButton_addScreenshots" << "pushButton_locateSteamDir", true);
        setStatusLabelText("analyzing queued screenshots", "");
        ui->progressBar_status->setValue(0);

        emit sendSelectedIDs(selectedUserID, selectedGameID, jpegQuality, this);

        // Anzeige aktualisieren
        QString gameID = ui->comboBox_gameID->currentText();
        controller->loadFirstScreenshotForGame(gameID);
        ui->horizontalScrollBarScreenshots->setValue(ui->horizontalScrollBarScreenshots->maximum()); // Nach Kopieren auf das neueste Bild im Ordner setzen

    }
}


void MainWindow::on_pushButton_prepare_clicked()
{
    emit pushButton_prepare_clicked();
}


void MainWindow::closeEvent(QCloseEvent *event)
{
    if (controller && controller->resetOnClose) {

        QString path;
        if (controller->settings)
            path = controller->settings->fileName();
        else
            path = QStandardPaths::writableLocation(QStandardPaths::AppDataLocation)
                   + "/Alsweider/SteaScreeLoaded.ini";

        // Ordner sichern, bevor Datei gelöscht wird
        QFileInfo info(path);
        QString dirPath = info.absolutePath();

        delete controller->settings;
        controller->settings = nullptr;

        bool removed = QFile::remove(path);
        qDebug() << "Settings-Datei gelöscht?" << removed << path;

        // Ordner prüfen und ggf. löschen
        QDir dir(dirPath);
        QStringList entries = dir.entryList(QDir::NoDotAndDotDot | QDir::AllEntries);

        if (entries.isEmpty()) {
            bool removedDir = dir.rmdir(dirPath);
            qDebug() << "Einstellungsordner war leer und wurde entfernt:" << removedDir << dirPath;
        } else {
            qDebug() << "Einstellungsordner nicht leer, verbleibt bestehen:" << dirPath;
        }

    } else {
        emit sendSettings(
            size(),
            pos(),
            ui->comboBox_userID->currentText().remove(QRegularExpression(" <.+>$")),
            ui->comboBox_gameID->currentText().remove(QRegularExpression(" <.+>$")),
            ui->spinBox_jpegQuality->value(),
            ui->comboBox_chooseAPI->currentIndex()
            );
    }

    event->accept();
}

// void MainWindow::closeEvent(QCloseEvent *event)
// {
//     if(controller && controller->resetOnClose) {
//         QString path;
//         if(controller->settings)
//             path = controller->settings->fileName();  // Pfad sichern, bevor settings gelöscht wird
//         else
//             path = QStandardPaths::writableLocation(QStandardPaths::AppDataLocation)
//                    + "/Alsweider/SteaScreeLoaded.ini"; // Fallback

//         delete controller->settings;
//         controller->settings = nullptr;

//         bool removed = QFile::remove(path);
//         qDebug() << "Settings-Datei gelöscht?" << removed << path;
//     } else {
//         emit sendSettings(
//             size(),
//             pos(),
//             ui->comboBox_userID->currentText().remove(QRegularExpression(" <.+>$")),
//             ui->comboBox_gameID->currentText().remove(QRegularExpression(" <.+>$")),
//             ui->spinBox_jpegQuality->value(),
//             ui->comboBox_chooseAPI->currentIndex()
//             );
//     }

//     event->accept();
// }

// void MainWindow::closeEvent(QCloseEvent *event)
// {
//     emit sendSettings(
//         size(),
//         pos(),
//       ui->comboBox_userID->currentText().remove(QRegularExpression(" <.+>$")),
//       ui->comboBox_gameID->currentText().remove(QRegularExpression(" <.+>$")),
//       ui->spinBox_jpegQuality->value(),
//        ui->comboBox_chooseAPI->currentIndex()
//         );
//     event->accept();

// }


void MainWindow::setController(Controller *ctrl)
{
    controller = ctrl;
    connect(controller, &Controller::sendPreviewImage,
            this, &MainWindow::showPreviewImage);
}


void MainWindow::setFooter()
{
    ui->labelFooter->clear();

    // Links klickbar machen
    ui->labelFooter->setTextFormat(Qt::RichText);
    ui->labelFooter->setTextInteractionFlags(Qt::TextBrowserInteraction);
    ui->labelFooter->setOpenExternalLinks(true);

    // Schriftfarbe
    ui->labelFooter->setStyleSheet("color: grey");

    // Zeit und Datum
    QTime now = QTime::currentTime();
    bool isNight = (now.hour() >= 20 || now.hour() < 6);
    QDate today = QDate::currentDate();
    bool isAdvent = (today.month() == 12 && today.day() >= 1 && today.day() <= 26);

    QStringList icons;

    // Grundmenge: Tages- oder Nachtzeichen
    if (isNight) {
        icons = { "🦉", "🌙" };
    } else {
        icons = {
            "🔔", "🏺", "★", "❤️", "✦", "🌱",
            "⭐", "🍃", "✨", "🌳", "⚜️", "❧", "❦",
            "✽", "❂", "🐝", "🍯", "🐻", "🐺", "🌞"
        };
    }

    // In der Adventszeit zusätzliche Weihnachtszeichen beimischen
    if (isAdvent) {
        icons << QStringList{ "🎄", "🎅", "❄️", "⛄", "🎁" };
    }

    // Sicherheit: niemals leere Liste
    if (icons.isEmpty()) {
        icons << "★";
    }

    // Zufälliges Icon
    int idx = QRandomGenerator::global()->bounded(icons.size());
    const QString iconDonations = icons.at(idx);

    // Farben (gold oder Advent-rot)
    QString iconColour = isAdvent ? "#B22222" : "#D4AF37";

    // Footer
    QString footer = QString(
                         "<span style='color: grey;'>"
                         "<a href='https://github.com/Alsweider/SteaScreeLoaded' style='color: inherit; text-decoration: none;'>%1</a> "
                         "v%2 by Alsweider, © 2025"
                         "</span>"
                         "<br>"
                         "<a href='https://ko-fi.com/alsweider' style='text-decoration: none;'>"
                         "<span style='color: %3;'>%4</span> "
                         "<span style='color: inherit;'>𝒮upport ₰evelopment</span>"
                         "</a>"
                         ).arg(progName, progVersion, iconColour, iconDonations);

    ui->labelFooter->setText(footer);
}


void MainWindow::openFolderInExplorer(const QString &path) {
    if (path.isEmpty()) {
        qDebug() << "MainWindow: Pfad ist leer!";
        return;
    }

#if defined(Q_OS_WIN)
    QProcess::startDetached("explorer", QStringList() << QDir::toNativeSeparators(path));
#elif defined(Q_OS_LINUX)
    QProcess::startDetached("xdg-open", QStringList() << path);
#elif defined(Q_OS_MACOS)
    QProcess::startDetached("open", QStringList() << path);
#endif
}


void MainWindow::on_pushButtonPreview_clicked()
{
    if(controller) {
        QString path = controller->getLastSelectedScreenshotPath(); // neuen Getter erstellen
        if(!path.isEmpty()) {
            controller->openPathInExplorer(path); // Explorer öffnen
        }
    }
}


void MainWindow::updatePreviewIcon()
{
    if (ui->pushButtonPreview && !m_previewOriginal.isNull())
    {
        QPixmap scaled = m_previewOriginal.scaled(
            ui->pushButtonPreview->size(),
            Qt::KeepAspectRatio,
            Qt::SmoothTransformation);

        ui->pushButtonPreview->setIcon(QIcon(scaled));
        ui->pushButtonPreview->setIconSize(scaled.size());
    }
}


void MainWindow::resizeEvent(QResizeEvent *event)
{
    QMainWindow::resizeEvent(event);
    updatePreviewIcon();
}


void MainWindow::on_splitter_splitterMoved(int pos, int index)
{
    Q_UNUSED(pos)
    Q_UNUSED(index)
    updatePreviewIcon();
}


void MainWindow::on_comboBox_gameID_currentTextChanged(const QString &arg1)
{
    checkModifiedGameList(arg1);

    // Titel der Screenshot-GroupBox aktualisieren
    QString title = arg1;

    // nur Spielname ohne ID anzeigen
    QRegularExpression re(R"(^\s*(\d+)\s*<([^>]+)>\s*$)");
    QRegularExpressionMatch match = re.match(arg1);
    if (match.hasMatch()) {
        title = match.captured(2); // nur der Name in < >
    }

    ui->groupBoxJpegQuality->setTitle(QString("Selected game: %1").arg(title));
}


void MainWindow::on_pushButtonSaveName_clicked()
{
    QString id = ui->lineEditGameID->text().trimmed();
    QString name = ui->lineEditGameName->text().trimmed();

    if (id.isEmpty())
        return;

    saveName(id, name);

    // ComboBox-Text aktualisieren
    int idx = ui->comboBox_gameID->currentIndex();
    if (!name.isEmpty())
        ui->comboBox_gameID->setItemText(idx, id + " <" + name + ">");
    else
        ui->comboBox_gameID->setItemText(idx, id);

    // QMessageBox::information(this, "Saved", "New name has been saved to local file: ids.txt");
    // Nachricht mit zusätzlichem Knopf
    QMessageBox msgBox(this);
    msgBox.setWindowTitle("Saved");
    msgBox.setText("New name has been saved to local file: ids.txt");

    msgBox.addButton(QMessageBox::Ok);
    QPushButton *openButton = msgBox.addButton("Open .txt", QMessageBox::ActionRole);

    msgBox.exec();

    if (msgBox.clickedButton() == openButton) {
        QFileInfo fi("ids.txt");
        if (fi.exists())
            QDesktopServices::openUrl(QUrl::fromLocalFile(fi.absoluteFilePath()));
    }
}


void MainWindow::checkModifiedGameList(const QString &arg1)
{
    static const QRegularExpression rx(R"(^\s*(\d+)(?:\s*<([^>]+)>)?\s*$)");
    QRegularExpressionMatch m = rx.match(arg1);
    if (!m.hasMatch())
        return;

    QString id = m.captured(1);
    QString name = m.captured(2);

    ui->lineEditGameID->setText(id);
    ui->lineEditGameID->setReadOnly(true);

    // Wenn kein Name in den spitzen Klammern steht:
    if (name.isEmpty()) {
        QMap<QString, QString> map = loadNameMap();
        if (map.contains(id)) {
            name = map.value(id);

            // -> ComboBox-Text automatisch ergänzen
            int idx = ui->comboBox_gameID->currentIndex();
            if (idx >= 0)
                ui->comboBox_gameID->setItemText(idx, id + " <" + name + ">");
        }
    }

    ui->lineEditGameName->setText(name);
}

// Datei mit gespeicherten ID->Name Paaren laden
QMap<QString, QString> MainWindow::loadNameMap() const
{
    QMap<QString, QString> map;
    QFile file("ids.txt");
    if (!file.open(QIODevice::ReadOnly | QIODevice::Text))
        return map;

    QTextStream in(&file);
    while (!in.atEnd()) {
        const QString line = in.readLine().trimmed();
        if (line.isEmpty() || !line.contains('='))
            continue;
        const QStringList parts = line.split('=');
        if (parts.size() == 2)
            map.insert(parts[0].trimmed(), parts[1].trimmed());
    }
    qDebug() << "ids.txt geladen: " << map;
    return map;
}


void MainWindow::saveName(const QString &id, const QString &name)
{
    QMap<QString, QString> map = loadNameMap();
    map[id] = name;

    QFile file("ids.txt");
    if (!file.open(QIODevice::WriteOnly | QIODevice::Text))
        return;

    QTextStream out(&file);
    for (auto it = map.begin(); it != map.end(); ++it)
        out << it.key() << "=" << it.value() << "\n";

    qDebug() << "ids.txt gespeichert: " << map;

}


// Spieleliste mit benutzerdefinierten Namen abgleichen
void MainWindow::updateComboBoxNames()
{
    QMap<QString, QString> map = loadNameMap();

    for (int i = 0; i < ui->comboBox_gameID->count(); ++i) {
        QString text = ui->comboBox_gameID->itemText(i);

        static const QRegularExpression rx(R"(^\s*(\d+)(?:\s*<([^>]+)>)?\s*$)");
        QRegularExpressionMatch m = rx.match(text);
        if (!m.hasMatch())
            continue;

        QString id = m.captured(1);
        QString name = m.captured(2);

        // Wenn kein Name vorhanden ist, prüfen wir die gespeicherte Liste
        if (name.isEmpty() && map.contains(id)) {
            name = map.value(id);
            ui->comboBox_gameID->setItemText(i, id + " <" + name + ">");
        }
    }
}


void MainWindow::showPreviewCount(int currentIndex, int totalCount)
{
    if (totalCount <= 0)
        ui->label_previewCount->setText("No Screenshots found");
    else
        ui->label_previewCount->setText(
            QString("Screenshot %1 of %2").arg(currentIndex).arg(totalCount));
}



void MainWindow::on_pushButtonApiKey_clicked()
{
    if (controller)
            controller->setApiKey(ui->lineEditApiKey->text());

    ui->lineEditApiKey->clear();
    ui->lineEditApiKey->setPlaceholderText("Key set, please restart.");
}


void MainWindow::on_comboBox_chooseAPI_currentIndexChanged(int index)
{
    qDebug() << "comboBox_chooseAPI Index gewählt: " << index;
    emit apiIndexChanged(index);

    //Notwendige Steuerelemente anzeigen/ausblenden
    showOrHideApiSettings(index);

    setStatusLabelText("Games source changed, please restart the programme.", warningColor);

    if(index == 2){
        ui->label_apiCheck->setText("●");
        ui->label_apiCheck->setToolTip("No connection needed.");

    } else {
        ui->label_apiCheck->setText("○");
        ui->label_apiCheck->setToolTip("Status unknown.");
    }
    ui->label_apiCheck->setStyleSheet("color: black; font-size: 12;");
}

void MainWindow::receiveApiKeyState(bool exists)
{
    if (exists) {
        ui->lineEditApiKey->setEchoMode(QLineEdit::Password);
        ui->lineEditApiKey->clear();
        ui->lineEditApiKey->setPlaceholderText("Key loaded");
    } else {
        ui->lineEditApiKey->setEchoMode(QLineEdit::Normal);
        ui->lineEditApiKey->clear();
        ui->lineEditApiKey->setPlaceholderText("Enter Steam API key...");
    }
}

void MainWindow::on_pushButtonClearKey_clicked()
{
    if(controller)
    controller->clearApiKey();
}


void MainWindow::on_pushButtonResetName_clicked()
{
    QString id = ui->lineEditGameID->text().trimmed();
    if (id.isEmpty())
        return;

    // Map laden
    QMap<QString, QString> map = loadNameMap();

    // ID entfernen
    if (map.contains(id))
        map.remove(id);

    // Map wieder in ids.txt schreiben
    QFile file("ids.txt");
    if (!file.open(QIODevice::WriteOnly | QIODevice::Text | QIODevice::Truncate))
        return;

    QTextStream out(&file);
    for (auto it = map.begin(); it != map.end(); ++it)
        out << it.key() << "=" << it.value() << "\n";

    file.close();

    // ComboBox-Eintrag zurücksetzen
    int idx = ui->comboBox_gameID->currentIndex();
    ui->comboBox_gameID->setItemText(idx, id);

    // LineEdit zurücksetzen
    ui->lineEditGameName->clear();

    // Nachricht für den Benutzer
    QMessageBox msgBox(this);
    msgBox.setWindowTitle("Name Reset");
    msgBox.setText("ID has been removed from ids.txt");

    msgBox.addButton(QMessageBox::Ok);
    QPushButton *openButton = msgBox.addButton("Open .txt", QMessageBox::ActionRole);

    msgBox.exec();

    if (msgBox.clickedButton() == openButton) {
        QFileInfo fi("ids.txt");
        if (fi.exists())
            QDesktopServices::openUrl(QUrl::fromLocalFile(fi.absoluteFilePath()));
    }
}


void MainWindow::on_horizontalScrollBarScreenshots_valueChanged(int value)
{
    if (controller)
        controller->setScreenshotIndex(value);
}


// Notwendige Steuerelemente anzeigen / ausblenden
void MainWindow::showOrHideApiSettings(int index){

    if (index == 1 || ui->comboBox_chooseAPI->currentIndex() == 1){
        ui->lineEditApiKey->setEnabled(true);
        ui->lineEditApiKey->setVisible(true);
        ui->pushButtonApiKey->setEnabled(true);
        ui->pushButtonApiKey->setVisible(true);
        ui->pushButtonClearKey->setEnabled(true);
        ui->pushButtonClearKey->setVisible(true);
        ui->label_GetKey->setEnabled(true);
        ui->label_GetKey->setVisible(true);
        ui->label_APIkey->setVisible(true);
        ui->label_APIkey->setEnabled(true);
    } else {
        ui->lineEditApiKey->setEnabled(false);
        ui->lineEditApiKey->setVisible(false);
        ui->pushButtonApiKey->setEnabled(false);
        ui->pushButtonApiKey->setVisible(false);
        ui->pushButtonClearKey->setEnabled(false);
        ui->pushButtonClearKey->setVisible(false);
        ui->label_GetKey->setEnabled(false);
        ui->label_GetKey->setVisible(false);
        ui->label_APIkey->setVisible(false);
        ui->label_APIkey->setEnabled(false);
    }

}


void MainWindow::onApiReachabilityChanged(bool erreichbar)
{
    if (erreichbar) {
        ui->label_apiCheck->setText("●");
        ui->label_apiCheck->setStyleSheet("color: green; font-size: 12;");
        ui->label_apiCheck->setToolTip("Connection established.");
    } else {
        ui->label_apiCheck->setText("○");
        ui->label_apiCheck->setStyleSheet("color: red; font-size: 12;");
        ui->label_apiCheck->setToolTip("Connection failed.");
    }
}


void MainWindow::on_actionReset_settings_triggered()
{

    if(controller) {
        controller->resetSettings();
    }
    bootStrap();

    ui->statusBar->showMessage("The settings file and the user interface have been reset.", 5000);
}


void MainWindow::on_actionOpen_settings_triggered()
{
    QString settingsPath = controller->settings->fileName();

    QFileInfo fileInfo(settingsPath);
    if(fileInfo.exists()) {
        // Pfad zum Settings-Ordner
        QDesktopServices::openUrl(QUrl::fromLocalFile(fileInfo.absolutePath()));
        ui->statusBar->showMessage("Settings folder opened.", 5000);
    } else {
        qWarning("SteaScreeLoaded.ini wurde nicht gefunden!");
        ui->statusBar->showMessage("Settings not found!", 5000);
    }
}


void MainWindow::on_action_Update_triggered()
{
    if (controller)
        controller->checkForUpdates();
}


void MainWindow::on_action_About_triggered()
{
    QDialog dialog(this);
    dialog.setWindowTitle("About SteaScreeLoaded");

    QVBoxLayout *layout = new QVBoxLayout(&dialog);

    // Haupttext
    QLabel *label = new QLabel(
        "<h3>SteaScreeLoaded " + progVersion + "</h3>"
                                               "<p>A Steam cloud screenshot upload helper.</p>"
                                               "<p>&copy; 2025 <a href=\"https://github.com/Alsweider\">Alsweider</a></p>"
                                               "<p><a href=\"https://github.com/Alsweider/SteaScreeLoaded\">SteaScreeLoaded</a> is a fork based on <a href=\"https://github.com/awthwathje/SteaScree\">SteaScree</a> by Foyl "
                                               "and is licensed under the "
                                               "<a href=\"https://www.gnu.org/licenses/gpl-3.0.html.en\">GNU GPL 3.0</a>.</p>"
                                               "<p>Developed in C++ and Qt Framework.</p>"
                                               "<p>Storage of API keys protected with <a href=\"https://github.com/kokke/tiny-AES-c\">tiny-AES-c</a> using AES-256-CBC encryption.</p>"
                                               "<hr>"
                                               "<p>If you enjoy using this software, you are welcome to support the author:</p>"
                                               "<p>Ko-Fi: <a href=\"https://ko-fi.com/alsweider\">https://ko-fi.com/alsweider</a></p>"
                                               "<p>Monero (XMR):</p>"
        );
    label->setTextFormat(Qt::RichText);
    label->setAlignment(Qt::AlignHCenter | Qt::AlignTop);
    label->setTextInteractionFlags(Qt::TextBrowserInteraction);   // Links klickbar
    label->setOpenExternalLinks(true);
    layout->addWidget(label);

    // --- Monero-Adresse ---
    const QString moneroAddress =
        "88o74DJuHyxNr8rFkbH2xaFKkN35jiUcS12htB13SNPVVrzA4zX4ruJj8AXURrR3ssMni8zeQZHAjV6aFYwNUZy8AwT5c8M";

    // --- QR-Button ---
    QPushButton *qrButton = new QPushButton;
    QPixmap qrPixmap(":/res/misc/moneroQR.png");

    qrButton->setIcon(qrPixmap);
    qrButton->setIconSize(QSize(150,150));
    qrButton->setFixedSize(160,160);               // schöner Rahmen
    qrButton->setFlat(true);                       // noch schöner
    qrButton->setCursor(Qt::PointingHandCursor);   // Handzeiger
    qrButton->setToolTip("Copy XMR address");

    layout->addWidget(qrButton, 0, Qt::AlignHCenter);

    // Klick auf QR-Button -> Monero-Adresse kopieren
    connect(qrButton, &QPushButton::clicked, this, [this, qrButton, moneroAddress](){
        QApplication::clipboard()->setText(moneroAddress);
        QToolTip::showText(qrButton->mapToGlobal(QPoint(0, qrButton->height())),
                           "XMR Address copied to clipboard");
    });

    // connect(qrButton, &QPushButton::clicked, this, [moneroAddress](){
    //     QClipboard *clipboard = QApplication::clipboard();
    //     clipboard->setText(moneroAddress);
    // });

    // Kopierfeld
    QTextEdit *moneroField = new QTextEdit(moneroAddress);
    moneroField->setReadOnly(true);
    moneroField->setLineWrapMode(QTextEdit::NoWrap);
    moneroField->setMaximumWidth(400);
    moneroField->setFixedHeight(50);

    QHBoxLayout *hLayout = new QHBoxLayout;
    hLayout->addStretch();
    hLayout->addWidget(moneroField);
    hLayout->addStretch();
    layout->addLayout(hLayout);

    // Schließen-Schaltfläche
    QPushButton *closeBtn = new QPushButton("Close");
    connect(closeBtn, &QPushButton::clicked, &dialog, &QDialog::accept);

    QHBoxLayout *buttonLayout = new QHBoxLayout;
    buttonLayout->addStretch();
    buttonLayout->addWidget(closeBtn);
    buttonLayout->addStretch();
    layout->addLayout(buttonLayout);

    dialog.exec();
}


void MainWindow::on_actionDelete_settings_triggered()
{
    controller->resetOnClose = true;
    ui->statusBar->showMessage("The settings file is removed upon closing the programme.", 5000);
}


bool MainWindow::isSteamRunning()
{
#ifdef Q_OS_WIN
    QStringList processNames = {"steam.exe", "steamwebhelper.exe", "steamservice.exe"};
    QProcess process;
    for (const QString &name : processNames) {
        process.start("tasklist", QStringList() << "/FI" << QString("IMAGENAME eq %1").arg(name));
        process.waitForFinished(1000); // Timeout 1s
        QString output = process.readAllStandardOutput();
        if (output.contains(name, Qt::CaseInsensitive)) {
            return true;
        }
    }
    return false;

#elif defined(Q_OS_LINUX)
    QStringList processNames = {"steam", "steamwebhelper"};
    QProcess process;
    for (const QString &name : processNames) {
        process.start("pgrep", QStringList() << name);
        process.waitForFinished();
        if (process.exitCode() == 0) {
            return true;
        }
    }
    return false;

#elif defined(Q_OS_MACOS)
    QStringList processNames = {"steam_osx", "steamwebhelper_osx"};
    QProcess process;
    for (const QString &name : processNames) {
        process.start("pgrep", QStringList() << name);
        process.waitForFinished();
        if (process.exitCode() == 0) {
            return true;
        }
    }
    return false;

#else
    // Unbekanntes System, kann nicht prüfen, Warnung erzwingen
    return true;
#endif
}



