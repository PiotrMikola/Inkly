#pragma once
#include <QtWidgets/QMainWindow>
#include <vector>
#include "Note.h"
#include "Settings.h"
#include "ui_Inkly.h"

class QLineEdit; class QListWidget; class QPlainTextEdit; class QTimer; class QLabel;

class Inkly : public QMainWindow
{
    Q_OBJECT

public:
    Inkly(QWidget* parent = nullptr);
    ~Inkly();
    void importFromFile(const QString& path, QWidget* parent);
    void bringToFront();
    void openExternal(const QString& path);
protected:
    void closeEvent(QCloseEvent* event) override;
    void dragEnterEvent(QDragEnterEvent* e) override;
    void dropEvent(QDropEvent* e) override;

private slots:
    void newNote();
    void deleteNote();
    void refreshList();
    void flushSave();
    void openSettings();
    void openFiles();

private:
    int  indexOfId(const QString& id) const;
    void applyTheme();
    void applySettings();
	void importFiles(const QStringList& files);
    void saveToFile(bool askPath);
    void updateItemLabel(const Note& n);
    void exportNotes(QWidget* parent);
    void importNotes(QWidget* parent);

    Ui::InklyClass ui;
    QLineEdit* searchEdit = nullptr;
    QListWidget* listWidget = nullptr;
    QLineEdit* titleEdit = nullptr;
    QPlainTextEdit* bodyEdit = nullptr;
    QTimer* saveTimer = nullptr;
    QLabel* posLabel = nullptr;
    QLabel* countLabel = nullptr;

    AppSettings       settings;
    std::vector<Note> notes;
    int  currentIndex = -1;
    bool dirty = false;
};