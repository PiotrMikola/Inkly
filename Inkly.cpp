#include "stdafx.h"
#include "Inkly.h"
#include "Storage.h"
#include "Settings.h"
#include "Theme.h"
#include "SettingsDialog.h"
#include "Note.h"
#include <Qt>
static QString listLabel(const Note& n)
{
	const QString t = n.title.isEmpty() ? QString("(bez tytułu)") : n.title;
	return n.unsavedToFile() ? "* " + t : t;
}
Inkly::Inkly(QWidget* parent)
    : QMainWindow(parent)
{
    ui.setupUi(this);
	for (QToolBar* tb : findChildren<QToolBar*>(QString(), Qt::FindDirectChildrenOnly))
		delete tb;
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
	titleEdit->setPlaceholderText("Tytuł");
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
	toolbar->setObjectName("mainToolbar");
	toolbar->setMovable(true);
	toolbar->setFloatable(false);
	toolbar->setContextMenuPolicy(Qt::PreventContextMenu);
	toolbar->setFont(iconFont);
	auto* actNew = toolbar->addAction(QString(QChar(0xE710)));
	actNew->setShortcut(QKeySequence::New);
	connect(actNew, &QAction::triggered, this, &Inkly::newNote);
	auto* actDelete = toolbar->addAction(QString(QChar(0xE74D)));
	actDelete->setShortcut(QKeySequence(Qt::CTRL | Qt::Key_Delete));
	connect(actDelete, &QAction::triggered, this, &Inkly::deleteNote);
	auto* actSave = toolbar->addAction(QString(QChar(0xE74E)));
	actSave->setToolTip("Zapisz jako plik (Ctrl+S)");
	actSave->setShortcut(QKeySequence::Save);
	connect(actSave, &QAction::triggered, this, [this] { saveToFile(false); });
	auto* actSaveAs = new QAction(this);
	actSaveAs->setShortcut(QKeySequence::SaveAs);        // Ctrl+Shift+S
	addAction(actSaveAs);
	connect(actSaveAs, &QAction::triggered, this, [this] { saveToFile(true); });
	auto* actOpen = toolbar->addAction(QString(QChar(0xE8E5)));
	actOpen->setToolTip("Otwórz plik (Ctrl+O)");
	actOpen->setShortcut(QKeySequence::Open);
	connect(actOpen, &QAction::triggered, this, &Inkly::openFiles);
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
	connect(toolbar, &QToolBar::orientationChanged, this, [spacer](Qt::Orientation o) {
		if (o == Qt::Horizontal)
			spacer->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Preferred);
		else
			spacer->setSizePolicy(QSizePolicy::Preferred, QSizePolicy::Expanding);
		});
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
	setAcceptDrops(true);                         
	bodyEdit->viewport()->setAcceptDrops(false);  
	QSettings s;
	restoreGeometry(s.value("geometry").toByteArray());
	restoreState(s.value("windowState").toByteArray());
}
Inkly::~Inkly()
{}
void Inkly::closeEvent(QCloseEvent* e)
{
    flushSave();
	QSettings s;
	s.setValue("geometry", saveGeometry());
	s.setValue("windowState", saveState());
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
		auto* item = new QListWidgetItem(listLabel(n));
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
	if (QMessageBox::question(this, "Usuń notatkę", "Czy na pewno chcesz usunąć wybraną notatkę?")
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
	saveAll(notes);
	updateItemLabel(n);

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
	dlg.onExport = [this](QWidget* p) { exportNotes(p); };
	dlg.onImport = [this](QWidget* p) { importNotes(p); };
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
	bodyEdit->setFont(f);
	bodyEdit->setLineWrapMode(settings.wordWrap ? QPlainTextEdit::WidgetWidth
		: QPlainTextEdit::NoWrap);

	saveTimer->setInterval(settings.autosaveMs);

	// panel boczny: pierwszy widget splittera
	if (auto* sp = qobject_cast<QSplitter*>(centralWidget()))
		sp->widget(0)->setVisible(settings.showSidebar);
}
void Inkly::saveToFile(bool askPath)
{
	if (currentIndex < 0) return;
	flushSave();
	Note& n = notes[currentIndex];

	QString path = n.filePath;
	if (askPath || path.isEmpty() || !QFileInfo(QFileInfo(path).absolutePath()).isDir()) {
		QString name = n.title;
		name.replace(QRegularExpression("[\\\\/:*?\"<>|]"), "_");
		if (name.isEmpty()) name = "notatka";

		QSettings s;
		const QString dir = n.filePath.isEmpty()
			? s.value("lastDir", QStandardPaths::writableLocation(QStandardPaths::DocumentsLocation)).toString()
			: QFileInfo(n.filePath).absolutePath();

		path = QFileDialog::getSaveFileName(this, "Zapisz jako", QDir(dir).filePath(name + ".md"),
			"Markdown (*.md);;Tekst (*.txt);;Wszystkie pliki (*.*)");
		if (path.isEmpty()) return;
		s.setValue("lastDir", QFileInfo(path).absolutePath());
	}

	QSaveFile f(path);
	if (!f.open(QIODevice::WriteOnly) || f.write(n.body.toUtf8()) < 0 || !f.commit()) {
		QMessageBox::warning(this, "Błąd", "Nie udało się zapisać pliku.");
		return;
	}
	n.filePath = path;
	n.exported = QDateTime::currentDateTime();
	saveAll(notes);
	updateItemLabel(n);
}

void Inkly::importNotes(QWidget* parent)
{
	flushSave();
	QSettings s;
	const QString dir = s.value("lastDir",
		QStandardPaths::writableLocation(QStandardPaths::DocumentsLocation)).toString();
	const QString path = QFileDialog::getOpenFileName(parent, "Importuj notatki", dir,
		"Kopia notatek Inkly (*.ink);;Wszystkie pliki (*.*)");
	if (path.isEmpty()) return;
	s.setValue("lastDir", QFileInfo(path).absolutePath());
	importFromFile(path, parent);
}

void Inkly::importFiles(const QStringList& paths)
{
	flushSave();
	int added = 0;
	for (const QString& p : paths) {
		QFile f(p);
		if (!f.open(QIODevice::ReadOnly)) continue;
		Note n;
		n.title = QFileInfo(p).completeBaseName();
		n.body = decodeText(f.readAll());
		n.filePath = p;
		n.exported = n.modified;
		notes.insert(notes.begin(), n);
		++added;
	}
	if (!added) return;
	saveAll(notes);
	searchEdit->clear();
	refreshList();
	listWidget->setCurrentRow(0);
}

void Inkly::bringToFront()
{
	setWindowState(windowState() & ~Qt::WindowMinimized);
	show();
	raise();
	activateWindow();
}
void Inkly::openFiles()
{
	QSettings s;
	const QString dir = s.value("lastDir",
		QStandardPaths::writableLocation(QStandardPaths::DocumentsLocation)).toString();
	const QStringList paths = QFileDialog::getOpenFileNames(this, "Otwórz pliki", dir,
		"Notatki (*.md *.markdown *.txt);;Wszystkie pliki (*.*)");
	if (paths.isEmpty()) return;
	s.setValue("lastDir", QFileInfo(paths.first()).absolutePath());
	importFiles(paths);
}
void Inkly::dragEnterEvent(QDragEnterEvent* e)
{
	if (e->mimeData()->hasUrls()) e->acceptProposedAction();
}

void Inkly::dropEvent(QDropEvent* e)
{
	QStringList texts;
	for (const QUrl& u : e->mimeData()->urls()) {
		if (!u.isLocalFile()) continue;
		const QString p = u.toLocalFile();
		const QString ext = QFileInfo(p).suffix().toLower();
		if (ext == "ink") importFromFile(p, this);
		else if (ext == "md" || ext == "markdown" || ext == "txt") texts << p;
	}
	if (!texts.isEmpty()) importFiles(texts);
}
void Inkly::updateItemLabel(const Note& n)
{
	const QString id = n.id.toString();
	for (int i = 0; i < listWidget->count(); ++i) {
		auto* item = listWidget->item(i);
		if (item->data(Qt::UserRole).toString() == id) {
			item->setText(listLabel(n));
			break;
		}
	}
}
void Inkly::exportNotes(QWidget* parent)
{
	flushSave();
	if (!saveAll(notes)) {
		QMessageBox::warning(parent, "Błąd", "Nie udało się zapisać notatek przed eksportem.");
		return;
	}

	const QString name = QDateTime::currentDateTime().toString("HHmmMMdd") + ".ink";
	QSettings s;
	const QString dir = s.value("lastDir",
		QStandardPaths::writableLocation(QStandardPaths::DocumentsLocation)).toString();

	const QString path = QFileDialog::getSaveFileName(parent, "Eksportuj notatki",
		QDir(dir).filePath(name), "Kopia notatek Inkly (*.ink)");
	if (path.isEmpty()) return;
	s.setValue("lastDir", QFileInfo(path).absolutePath());

	if (QFile::exists(path)) QFile::remove(path);   // QFile::copy nie nadpisuje, a pytanie o to zadał już dialog
	if (!QFile::copy(dataPath(), path))
		QMessageBox::warning(parent, "Błąd", "Nie udało się zapisać pliku eksportu.");
	else
		QMessageBox::information(parent, "Eksport", QString("Wyeksportowano %1 notatek.").arg(notes.size()));
}
void Inkly::importFromFile(const QString& path, QWidget* parent)
{
	flushSave();
	QFile f(path);
	if (!f.open(QIODevice::ReadOnly)) {
		QMessageBox::warning(parent, "Błąd", "Nie udało się otworzyć pliku.");
		return;
	}
	QJsonParseError err;
	const QJsonDocument doc = QJsonDocument::fromJson(f.readAll(), &err);
	if (err.error != QJsonParseError::NoError || !doc.isArray()) {
		QMessageBox::warning(parent, "Błąd", "To nie jest poprawny plik z notatkami Inkly.");
		return;
	}

	int added = 0, copies = 0, skipped = 0;
	for (const QJsonValue& v : doc.array()) {
		if (!v.isObject()) continue;
		Note n = fromJson(v.toObject());

		const int idx = indexOfId(n.id.toString());
		if (idx >= 0) {
			if (notes[idx].title == n.title && notes[idx].body == n.body) { ++skipped; continue; }
			n.id = QUuid::createUuid();      // inna wersja tej samej notatki, więc zostaje kopia
			++copies;
		}
		else {
			++added;
		}

		// powiązanie z plikiem ma sens tylko, jeśli ten plik istnieje na tym komputerze
		if (!QFileInfo(n.filePath).isFile()) { n.filePath.clear(); n.exported = QDateTime(); }

		notes.push_back(n);                  // na koniec, więc indeksy istniejących notatek się nie zmieniają
	}

	if (added + copies == 0) {
		QMessageBox::information(parent, "Import", "Brak nowych notatek do zaimportowania.");
		return;
	}
	saveAll(notes);

	// odśwież listę i zachowaj zaznaczenie
	const QString curId = currentIndex >= 0 ? notes[currentIndex].id.toString() : QString();
	refreshList();
	int row = 0;
	for (int i = 0; i < listWidget->count(); ++i)
		if (listWidget->item(i)->data(Qt::UserRole).toString() == curId) { row = i; break; }
	if (listWidget->count() > 0) listWidget->setCurrentRow(row);

	QMessageBox::information(parent, "Import",
		QString("Dodano: %1\nDodano jako kopie (inna wersja): %2\nPominięto identyczne: %3")
		.arg(added).arg(copies).arg(skipped));
}
void Inkly::openExternal(const QString& path)
{
	const QString ext = QFileInfo(path).suffix().toLower();
	if (ext == "ink") { importFromFile(path, this); return; }
	if (ext != "md" && ext != "markdown" && ext != "txt") return;

	// jeśli ten plik już jest notatką, zaznacz ją zamiast tworzyć duplikat
	const QString abs = QDir::cleanPath(QFileInfo(path).absoluteFilePath());
	for (const Note& n : notes) {
		if (n.filePath.isEmpty()) continue;
		if (QDir::cleanPath(n.filePath).compare(abs, Qt::CaseInsensitive) != 0) continue;
		searchEdit->clear();
		for (int r = 0; r < listWidget->count(); ++r)
			if (listWidget->item(r)->data(Qt::UserRole).toString() == n.id.toString()) {
				listWidget->setCurrentRow(r);
				break;
			}
		return;
	}
	importFiles({ path });
}