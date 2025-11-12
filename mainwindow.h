#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include "qtreewidgetdraganddrop.h"

#include <QMainWindow>
#include <QTreeWidgetItem>
#include <QMessageBox>
#include <QPushButton>
#include <QMovie>
#include <QComboBox>
#include <QDesktopServices>
#include <QUrl>
#include <QProcess>

class Controller;

namespace Ui {
class MainWindow;
}


class MainWindow : public QMainWindow
{
    Q_OBJECT


public:
    explicit MainWindow(QWidget *parent = 0);
    ~MainWindow();
    void bootStrap();
    void setController(Controller *ctrl);


protected:
    void closeEvent(QCloseEvent *event) Q_DECL_OVERRIDE;
    void resizeEvent(QResizeEvent *event) override;


private:
    Ui::MainWindow *ui;
    QMovie *gifLoader = new QMovie("://res/misc/loader.gif");
    QComboBox *userIDComboBox;
    void makeWideMessageBox(QMessageBox *msgBox, quint32 width);
    void disableAllControls();
    const QString warningColor = "#ab4e52";
    Controller *controller = nullptr;
    void setFooter();
    QPixmap m_previewOriginal;
    void updatePreviewIcon();
    void checkModifiedGameList(const QString &arg1);
    QMap<QString, QString> loadNameMap() const;
    void saveName(const QString &id, const QString &name);
    void updateComboBoxNames();
    QString progVersion = APP_VERSION;
    QString progName = APP_NAME;


signals:
    void sendButtonList(QList<QPushButton*> buttonList);
    void pushButton_addScreenshots_clicked();
    void pushButton_prepare_clicked();
    void clearScreenshotPathsPool();
    void clearState();
    void sendSelectedIDs(QString selectedUserID, QString selectedGameID, quint32 jpegQuality, MainWindow *mainWindow);
    void getSteamDir();
    void sendUserDataPaths(QString steamDir);
    void clearCopyingStatusLabels();
    void writeVDF();
    void getVDFStatus();
    void sendSettings(QSize size, QPoint pos, QString userID, QString userIDComboBox, quint32 jpegQuality);
    void sendScreenshotsSelected(QStringList screenshotsSelected);
    void sendNeverOfferUpdate();
    void sendNewlySelectedUserID(QString userID);
    void sendTreeWidgetPointer(QTreeWidgetDragAndDrop *treeWidget);
    void sendNewlySelectedGameID(QString gameID);



public slots:
    void addWidgetItemToScreenshotList(QTreeWidgetItem *item);
    void resizeScreenshotListColumns();
    void setWidgetsDisabled(QStringList list, bool disable);
    void setProgressBarLength(quint32 length);
    void locateSteamDir(QString steamDir);
    void prepareScreenshots(quint32 addedLines);
    void moveWindow(QSize geometry, QPoint moveToPoint);
    void setComboBoxesCleared(QStringList list);
    void setLabelsCleared(QStringList list);
    void insertIntoComboBox(QString name, QStringList items);
    void setLabelsOnMissingStuff(bool userDataMissing, QString vdfFilename);
    void returnScreenshotsSelected(QString lastSelectedScreenshotDir);
    void setProgressBarValue(quint32 value);
    void deleteCopiedWidgetItem(QString path);
    void setIndexOfComboBox(QString name, QString text);
    void setLabelsText(QStringList list, QString text);
    void setLabelsVisible(QStringList list, bool visible);
    void setStatusLabelText(QString text, QString color);
    void setDirStatusLabelsVisible(bool visible);
    void offerUpdate(QString version, QString link);
    void setJpegQualityValue(quint32 jpegQualityValue);
    void showPreviewImage(QPixmap pixmap);
    void onGameSelected(QString gameID);
    void openFolderInExplorer(const QString &path);
    void showPreviewCount(int currentIndex, int totalCount);


private slots:
    void on_pushButton_addScreenshots_clicked();
    void on_pushButton_clearQueue_clicked();
    void on_pushButton_copyScreenshots_clicked();
    void on_pushButton_prepare_clicked();
    void on_pushButton_locateSteamDir_clicked();
    void reactToComboBoxActivation(QString userID);
    void on_pushButtonPreview_clicked();
    void on_splitter_splitterMoved(int pos, int index);
    void on_comboBox_gameID_currentTextChanged(const QString &arg1);
    void on_pushButtonSaveName_clicked();
    void on_pushButtonLeft_clicked();
    void on_pushButtonRight_clicked();
};

#endif // MAINWINDOW_H
