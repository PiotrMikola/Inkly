#pragma once
#include "Note.h"
#include "stdafx.h"

inline QJsonObject toJson(const Note& n)
{
	return {
		{ "id", n.id.toString(QUuid::WithoutBraces) },
		{ "title", n.title },
		{ "body", n.body },
		{ "created", n.created.toString(Qt::ISODate) },
		{ "modified", n.modified.toString(Qt::ISODate) }
	};
}
inline Note fromJson(const QJsonObject& o)
{
	Note n;
    const QString idStr = o.contains("id") ? o["id"].toString() : o["id "].toString();
	n.id = QUuid::fromString(idStr);
    if (n.id.isNull())
		n.id = QUuid::createUuid();
	n.title = o["title"].toString();
	n.body = o["body"].toString();
	n.created = QDateTime::fromString(o["created"].toString(), Qt::ISODate);
	n.modified = QDateTime::fromString(o["modified"].toString(), Qt::ISODate);
	return n;
}
inline QString dataPath()
{
    const QString dir = QStandardPaths::writableLocation(QStandardPaths::AppDataLocation);
    QDir().mkpath(dir);
    return dir + "/notes.json";
}

inline bool saveAll(const std::vector<Note>& notes)
{
    QJsonArray arr;
    for (const Note& n : notes)
        arr.append(toJson(n));
    QSaveFile file(dataPath());
    if (!file.open(QIODevice::WriteOnly))
        return false;
    file.write(QJsonDocument(arr).toJson(QJsonDocument::Indented));
    return file.commit();
}

inline std::vector<Note> loadAll()
{
    std::vector<Note> result;
    QFile file(dataPath());
    if (!file.open(QIODevice::ReadOnly))
        return result;   // pierwszy start = brak pliku, to normalne
    const QJsonDocument doc = QJsonDocument::fromJson(file.readAll());
    const QJsonArray arr = doc.array();
    for (const QJsonValue& v : arr)
        result.push_back(fromJson(v.toObject()));
    return result;
}