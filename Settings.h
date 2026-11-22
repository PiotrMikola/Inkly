#pragma once
#include "stdafx.h"
enum class ThemeMode { System = 0, Light = 1, Dark = 2 };
struct AppSettings {
    ThemeMode theme = ThemeMode::System;
    QColor    accent = QColor("#0078D4");
    QString   fontFamily = "Cascadia Code";
    int       fontSize = 14;
    bool      wordWrap = true;
    bool      showSidebar = true;
    int       autosaveMs = 800;
    void load()
    {
        QSettings s;
        theme = static_cast<ThemeMode>(s.value("theme", 0).toInt());
        accent = QColor(s.value("accent", "#0078D4").toString());
        fontFamily = s.value("fontFamily", fontFamily).toString();
        fontSize = s.value("fontSize", fontSize).toInt();
        wordWrap = s.value("wordWrap", wordWrap).toBool();
        showSidebar = s.value("showSidebar", showSidebar).toBool();
        autosaveMs = s.value("autosaveMs", autosaveMs).toInt();
    }
    void save() const
    {
        QSettings s;
        s.setValue("theme", static_cast<int>(theme));
        s.setValue("accent", accent.name());
        s.setValue("fontFamily", fontFamily);
        s.setValue("fontSize", fontSize);
        s.setValue("wordWrap", wordWrap);
        s.setValue("showSidebar", showSidebar);
        s.setValue("autosaveMs", autosaveMs);
    }
};