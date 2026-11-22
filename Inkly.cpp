#include "stdafx.h"
#include "Inkly.h"
#include "Storage.h"
#include "Settings.h"
#include "Theme.h"
#include "SettingsDialog.h"
Inkly::Inkly(QWidget* parent)
    : QMainWindow(parent)
{
    ui.setupUi(this);
	QFont iconFont;
	iconFont.setFamilies({ "Segoe Fluent Icons", "Segoe MDL2 Assets" });
	iconFont.setPixelSize(16);
	//lewa czesc
    auto* left = new QWidget;
	auto* leftLayout = new QVBoxLayout(left);
    searchEdit = new QLineEdit;
	searchEdit->setPlaceholderText("Wyszukaj...");
    listWidget = new QListWidget;
	leftLayout->addWidget(searchEdit);
	leftLayout->addWidget(listWidget);
	left->setObjectName("sidebar");
	left->setAttribute(Qt::WA_StyledBackground, true);
	leftLayout->setContentsMargins(10, 10, 10, 10);
	leftLayout->setSpacing(8);
	//prawa czesc
	auto* right = new QWidget;
	auto* rightLayout = new QVBoxLayout(right);
	titleEdit = new QLineEdit;
	titleEdit->setPlaceholderText("Tytul");
	bodyEdit = new QPlainTextEdit;
	rightLayout->addWidget(titleEdit);
	rightLayout->addWidget(bodyEdit);
	titleEdit->setObjectName("title");
	rightLayout->setContentsMargins(0, 0, 0, 0);
	rightLayout->setSpacing(0);
	bodyEdit->setFrameShape(QFrame::NoFrame);
	bodyEdit->document()->setDocumentMargin(16);
	//splitter
	auto* splitter = new QSplitter(this);
	splitter->addWidget(left);
	splitter->addWidget(right);
	splitter->setStretchFactor(0, 1);
	splitter->setStretchFactor(1, 3);
	setCentralWidget(splitter);
	splitter->setHandleWidth(1);
	splitter->setChildrenCollapsible(false);
	//autosave
	saveTimer = new QTimer(this);
	saveTimer->setSingleShot(true);
	saveTimer->setInterval(800);
	connect(saveTimer, &QTimer::timeout, this, &Inkly::flushSave);
	auto markDirty = [this]{ dirty = true; saveTimer->start(); };
	connect(titleEdit, &QLineEdit::textChanged, this, markDirty);
	connect(bodyEdit, &QPlainTextEdit::textChanged, this, markDirty);
	//toolbar
	auto* toolbar = addToolBar("Pasek narzędzi");
	toolbar->setMovable(false);
	toolbar->setFloatable(false);
	toolbar->setFont(iconFont);
	auto* actNew = toolbar->addAction(QString(QChar(0xE710)));
	actNew->setShortcut(QKeySequence::New);
	connect(actNew, &QAction::triggered, this, &Inkly::newNote);
	auto* actDelete = toolbar->addAction(QString(QChar(0xE74D)));
	actDelete->setShortcut(QKeySequence(Qt::CTRL | Qt::Key_Delete));
	connect(actDelete, &QAction::triggered, this, &Inkly::deleteNote);
	auto* actFind = new QAction(this);
	actFind->setShortcut(QKeySequence::Find);
	addAction(actFind);
	connect(actFind, &QAction::triggered, this, [this] { searchEdit->setFocus(); });
	//wybor notatki
	connect(listWidget, &QListWidget::currentItemChanged, this,
		[this](QListWidgetItem* item, QListWidgetItem*) {
			flushSave();
			if (!item) { currentIndex = -1; return; }
			currentIndex = indexOfId(item->data(Qt::UserRole).toString());
			if (currentIndex < 0) return;
			QSignalBlocker b1(titleEdit), b2(bodyEdit);
			titleEdit->setText(notes[currentIndex].title);
			bodyEdit->setPlainText(notes[currentIndex].body);
		});
	connect(searchEdit, &QLineEdit::textChanged, this, &Inkly::refreshList);
	//start
	notes = loadAll();
	refreshList();
	if (listWidget->count() > 0)
		listWidget->setCurrentRow(0);
	//theme
	auto* actSidebar = toolbar->addAction(QString(QChar(0xE700)));
	actSidebar->setToolTip("Panel boczny");
	auto* spacer = new QWidget;
	spacer->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Preferred);
	toolbar->addWidget(spacer);
	auto* actSettings = toolbar->addAction(QString(QChar(0xE713)));  // koło zębate
	actSettings->setToolTip("Ustawienia (Ctrl+,)");
	actSettings->setShortcut(QKeySequence("Ctrl+,"));
	connect(actSidebar, &QAction::triggered, this, [this] {
    settings.showSidebar = !settings.showSidebar;
	settings.save();
    applySettings();
});
	connect(actSettings, &QAction::triggered, this, &Inkly::openSettings);

	settings.load();
	applySettings();
}
Inkly::~Inkly()
{}
void Inkly::closeEvent(QCloseEvent* e)
{
    flushSave();
	e->accept();
}
int Inkly::indexOfId(const QString& id) const
{
	for (size_t i = 0; i < notes.size(); ++i)
		if (notes[i].id.toString() == id)
			return static_cast<int>(i);
	return -1;
}
void Inkly::refreshList()
{
	const QString filter = searchEdit->text().trimmed();
	QSignalBlocker block(listWidget);
	listWidget->clear();
	for (const Note& n : notes) {
		const bool match = filter.isEmpty()
			|| n.title.contains(filter, Qt::CaseInsensitive)
			|| n.body.contains(filter, Qt::CaseInsensitive);
		if (!match) continue;
		auto* item = new QListWidgetItem(n.title.isEmpty() ? "(bez tytulu)" : n.title);
		item->setData(Qt::UserRole, n.id.toString());
		listWidget->addItem(item);
		}
		
}
void Inkly::newNote()
{
	flushSave();
	Note n;
	n.title = "Nowa notatka"; 
	notes.insert(notes.begin(), n);
	saveAll(notes);
	searchEdit->clear();
	refreshList();
	listWidget->setCurrentRow(0);
	titleEdit->setFocus();
	titleEdit->selectAll();
}
void Inkly::deleteNote()
{
	if (currentIndex < 0) return;
	if (QMessageBox::question(this, "Usun notatke", "Czy na pewno chcesz usunac wybrana notatke?")
			!= QMessageBox::Yes) 
		return;
	saveTimer->stop();
	dirty = false;
	notes.erase(notes.begin() + currentIndex);
	currentIndex = -1;
	saveAll(notes);
	{
		QSignalBlocker b1(titleEdit), b2(bodyEdit);
		titleEdit->clear();
		bodyEdit->clear();
	}
	refreshList();
}
void Inkly::flushSave()
{
	saveTimer->stop();
	if (!dirty || currentIndex < 0 || currentIndex >= static_cast<int>(notes.size()))
		return;
	dirty = false;
	Note& n = notes[currentIndex];
	n.title = titleEdit->text();
	n.body = bodyEdit->toPlainText();
	n.modified = QDateTime::currentDateTime();
	saveAll(notes);
	const QString id = n.id.toString();
	for (int i = 0; i < listWidget->count(); ++i) {
		auto* item = listWidget->item(i);
		if (item->data(Qt::UserRole).toString() == id) {
			item->setText(n.title.isEmpty() ? "(bez tytulu)" : n.title);
			break;
		}
	}
}
void Inkly::applyTheme()
{
	auto* hints = QGuiApplication::styleHints();
	switch (settings.theme) {
	case ThemeMode::Light: hints->setColorScheme(Qt::ColorScheme::Light); break;
	case ThemeMode::Dark: hints->setColorScheme(Qt::ColorScheme::Dark); break;
	default: hints->setColorScheme(Qt::ColorScheme::Unknown); break;
	}
	const bool dark = hints->colorScheme() == Qt::ColorScheme::Dark;
	const QString iconFont = QFontDatabase::families().contains("Segoe Fluent Icons")
		? "Segoe Fluent Icons" : "Segoe MDL2 Assets";
	qApp->setStyleSheet(buildStyle(dark ? darkPalette() : lightPalette(), settings.accent, iconFont));
}
void Inkly::openSettings()
{
	SettingsDialog dlg(settings, this);
	if (dlg.exec() == QDialog::Accepted) {
		settings = dlg.value();
		settings.save();
		applySettings();
	}
}
void Inkly::applySettings()
{
	applyTheme();

	QFont f(settings.fontFamily, settings.fontSize);
	f.setStyleHint(QFont::Monospace);
	bodyEdit->setFont(f);
	bodyEdit->setLineWrapMode(settings.wordWrap ? QPlainTextEdit::WidgetWidth
		: QPlainTextEdit::NoWrap);

	saveTimer->setInterval(settings.autosaveMs);
	statusBar()->setVisible(settings.showStatusBar);

	// panel boczny: pierwszy widget splittera
	if (auto* sp = qobject_cast<QSplitter*>(centralWidget()))
		sp->widget(0)->setVisible(settings.showSidebar);
}