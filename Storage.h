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
        { "modified", n.modified.toString(Qt::ISODateWithMs) },
        { "filePath", n.filePath },
        { "exported", n.exported.toString(Qt::ISODateWithMs) }
    };
}

inline Note fromJson(const QJsonObject& o)
{
    Note n;
    const QString idStr = o.contains("id") ? o["id"].toString() : o["id "].toString();
    n.id = QUuid::fromString(idStr);
    if (n.id.isNull()) n.id = QUuid::createUuid();
    n.title = o["title"].toString();
    n.body = o["body"].toString();
    n.created = QDateTime::fromString(o["created"].toString(), Qt::ISODate);
    n.modified = QDateTime::fromString(o["modified"].toString(), Qt::ISODateWithMs);
    n.filePath = o["filePath"].toString();
    n.exported = QDateTime::fromString(o["exported"].toString(), Qt::ISODateWithMs);
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
inline QString decodeText(const QByteArray& data)
{
    QString s;
    if (auto enc = QStringConverter::encodingForData(data)) {      // jest BOM (UTF-8/16)
        QStringDecoder d(*enc);
        s = d(data);
    }
    else {
        QStringDecoder utf8(QStringConverter::Utf8);
        s = utf8(data);
        if (utf8.hasError())                                       // to nie UTF-8, więc ANSI
            s = QString::fromLocal8Bit(data);
    }
    if (s.startsWith(QChar(0xFEFF))) s.remove(0, 1);
    s.replace("\r\n", "\n");
    return s;
}