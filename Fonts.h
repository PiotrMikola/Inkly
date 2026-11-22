#pragma once
#include "stdafx.h"
inline QString fontsDir()
{
    const QString d = QStandardPaths::writableLocation(QStandardPaths::AppDataLocation) + "/fonts";
    QDir().mkpath(d);
    return d;
}
inline void loadUserFonts()
{
    const QDir dir(fontsDir());
    for (const QString& f : dir.entryList({ "*.ttf", "*.otf" }, QDir::Files))
        QFontDatabase::addApplicationFont(dir.filePath(f));
}
inline QString installFont(const QString& src)
{
    const QString dst = QDir(fontsDir()).filePath(QFileInfo(src).fileName());
    if (!QFile::exists(dst) && !QFile::copy(src, dst)) return {};
    const int id = QFontDatabase::addApplicationFont(dst);
    if (id < 0) return {};
    const QStringList fams = QFontDatabase::applicationFontFamilies(id);
    return fams.isEmpty() ? QString() : fams.first();
}