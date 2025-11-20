#ifndef CONTROLLER_H
#define CONTROLLER_H

#include "largefiledialog.h"
#include "mainwindow.h"

#include <QObject>
#include <QSettings>
#include <QTreeWidgetItem>
#include <QNetworkReply>
#include <QPushButton>
#include <QFile>


class Controller : public QObject
{
    Q_OBJECT


public:
    explicit Controller(QObject *parent = 0);
    void bootStrap();
    QString getLastSelectedScreenshotPath() const;
    void resetSettings();
    bool resetOnClose = false;
    QSettings *settings;
    void checkForUpdates();


private:
    void readSettings();
    QString convertSlashes(QString str);
    QStringList readVDF();
    void populateScreenshotQueue(QStringList screenshotPathsList);
    bool isUnixLikeOS;
    const QString vdfFilename = "screenshots.vdf";
    QString selectedUserID;
    QString selectedGameID;
    QString userDataDir;
    QString defaultSteamDir;
    QString steamDir;
    QStringList vdfPaths;
    QString userID;
    QString someID;
    QHash<QString, QString> games;
    QStringList screenshotPathsPool;
    QStringList lines;
    QString lastSelectedScreenshotDir;
    QString lastSelectedUserID;
    QString lastSelectedGameID;
    quint32 selectedJpegQuality;
    QStringList copiedGames;
    QString copyDest;
    qint32 opening;
    qint32 closing;
    qint32 lastEntryValue;
    quint32 copiedScreenshotsNum = 0;
    quint32 copiedDirsToNum = 0;
    quint32 addedLines = 0;
    LargeFileDialog *largeFileDialog;
    QList<Screenshot> preparedScreenshotList;
    const quint32 steamMaxSideSize = 16000;
    const quint32 steamMaxResolution = 26210175;
    QString offerUpdateSetting;
    QTreeWidgetDragAndDrop *treeWidget;
    const QStringList imageFormatsSupported = QStringList() << "jpg" << "jpeg" << "png" << "bmp" << "tif" << "tiff";
    bool someScreenshotsWereNotPrepared;
    void pushScreenshots(QList<Screenshot> screenshotList);
    void resizeAndSaveLargeScreenshot(Screenshot screenshot);
    void getUserDecisionAboutLargeScreenshots(QList<Screenshot> screenshotList, MainWindow *mainWindow);
    void saveThumbnail(QString filename, QImage image, quint32 width, quint32 height);
    QString getPersonalNameByUserID(QString userID);
    void getShortcutNames();
    QString getEncodingProcessOfJpeg(QFile *file);
    const QString warningColor = "#ab4e52";
    const quint32 defaultJpegQuality = 100;
    QString currentUserID;
    QString lastSelectedScreenshotPath;
    QStringList m_screenshotFiles;
    int m_currentScreenshotIndex = 0;
    QString apiKey;
    int apiIndex = 1;
    void checkApiReachability(const QUrl &url);
    QString loadJsonLocal();
    void loadJsonFromGithub();
    void parseJson(const QByteArray &raw);
    QString encryptAPIKey(const QString &plain);
    QString decryptAPIKey(const QString &enc);

    // Hauptschlüssel zur AES-256-Verschlüsselung
    static constexpr uint8_t AES_KEY[32] = {
        0x86, 0xEE, 0xDF, 0xA1, 0x27, 0xB3, 0x1C,
        0x82, 0x87, 0xE0, 0x95, 0xA0, 0x64, 0xD5,
        0x88, 0xDE, 0xF6, 0x8D, 0x24, 0xDB, 0xBD,
        0xD9, 0xC9, 0xA0, 0x09, 0xD6, 0xF3, 0x70,
        0xA4, 0x4C, 0x81, 0x88
    };

    // Initialisierungsvektor
    static constexpr uint8_t AES_IV[16] = {
        0x34, 0x19, 0xD3, 0xCE, 0xAC, 0xF4, 0xFB,
        0x70, 0x11, 0xB8, 0xDC, 0x19, 0xB7, 0xD8,
        0x4A, 0xAF
    };

#if defined(Q_OS_WIN32)
    const QString os = "Windows";
#elif defined(Q_OS_LINUX)
    const QString os = "Linux";
#elif defined(Q_OS_OSX)
    const QString os = "macOS";
#endif


signals:
    void adjustButtons(QList<QPushButton*> buttonList, QString os);
    void addWidgetItemToScreenshotList(QTreeWidgetItem *item);
    void resizeScreenshotListColumns();
    void sendProgressBarLength(quint32 length);
    void sendSteamDir(QString steamDir);
    void sendLinesState(quint32 addedLines);
    void moveWindow(QSize geometry, QPoint moveToPoint);
    void setLabelStatusErrorVisible(bool visible);
    void sendWidgetsDisabled(QStringList list, bool disable);
    void sendLabelsCleared(QStringList list);
    void sendLabelsVisible(QStringList list, bool visible);
    void sendComboBoxesCleared(QStringList list);
    void sendLabelsOnMissingStuff(bool userDataMissing, QString vdfFilename);
    void sendLastSelectedScreenshotDir(QString lastSelectedScreenshotDir);
    void sendProgressBarValue(quint32 value);
    void deleteCopiedWidgetItem(QString path);
    void sendToComboBox(QString name, QStringList items);
    void sendIndexOfComboBox(QString name, QString text);
    void sendIndexOfComboBoxAPI(QString name, int index);
    void sendLabelsText(QStringList list, QString text);
    void sendScreenshotList(QList<Screenshot> screenshotList, QPoint center, QStringList steamLimits);
    void sendStatusLabelText(QString text, QString color);
    void setupStatusArea(quint32 progressBarMaximum);
    void sendDirStatusLabelsVisible(bool visible);
    void sendUpdateInfo(QString version, QString link);
    void sendJpegQualityValue(quint32 jpegQualityValue);
    void openPathInExplorer(const QString &path);
    void sendPreviewImage(QPixmap pixmap);
    void sendPreviewCount(int currentIndex, int totalCount);    
    void sendApiKeyState(bool keyExists);
    void sendApiIndex(int index);
    void apiReachabilityChanged(bool erreichbar);
    //void settingsReset();
    void updateStatusMessage(const QString &message);


public slots:
    void getButtonList(QList<QPushButton *> buttonList);
    void writeSettings(QSize size, QPoint pos, QString userID, QString gameID, quint32 jpegQuality, int apiIndex);
    void removeEntryFromScreenshotPathsPool(QString entry);
    void returnLastSelectedScreenshotDir();
    void clearScreenshotPathsPool();
    void clearState();
    void prepareScreenshots(QString userID, QString gameID, quint32 jpegQuality, MainWindow *mainWindow);
    void setUserDataPaths(QString dir);
    void returnSteamDir();
    void writeVDF();
    void returnLinesState();
    void clearCopyingStatusLabels();
    void addScreenshotsToPool(QStringList screenshotsSelected);
    void prepareScreenshotListWithDecisions(QList<Screenshot> screenshotList);
    void writeSettingNeverOfferUpdate();
    void fillGameIDs(QString userIDCombined);
    void receiveTreeWidgetPointer(QTreeWidgetDragAndDrop *receivedWidget);
    void loadFirstScreenshotForGame(QString gameID);
    void onUserIDSelected(const QString &userID);
    // void showNextScreenshot();
    // void showPreviousScreenshot();
    void setApiKey(QString key);
    void setApiIndex(int index);
    void clearApiKey();
    void setScreenshotIndex(int index);


private slots:
    void handleUpdate(QNetworkReply *reply);
    void getGameNamesV2(QNetworkReply *reply);
    void getGameNamesV1(QNetworkReply *reply);


};

#endif // CONTROLLER_H
