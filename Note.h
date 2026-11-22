#pragma once
#include "stdafx.h"

struct Note{ 
	QUuid id = QUuid::createUuid();
	QString title;
	QString body;
	QDateTime created = QDateTime::currentDateTime();
	QDateTime modified = created;
};