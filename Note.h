#pragma once
#include "stdafx.h"

struct Note {
	QUuid id = QUuid::createUuid();
	QString title;
	QString body;
	QDateTime created = QDateTime::currentDateTime();
	QDateTime modified = created;
	QString   filePath;     // plik powiązany z notatką (pusty = brak)
	QDateTime exported;     // ostatni zapis do pliku (pusty = nigdy)

	bool unsavedToFile() const { return exported.isNull() || modified > exported; }
};