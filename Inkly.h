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

protected:
    void closeEvent(QCloseEvent* event) override;

private slots:
    void newNote();
    void deleteNote();
    void refreshList();
    void flushSave();
    void openSettings();

private:
    int  indexOfId(const QString& id) const;
    void applyTheme();
    void applySettings();

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