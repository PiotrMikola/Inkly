#pragma once
#include "stdafx.h"

struct Palette {
    QString bg, panel, text, sub, border, hover, selected;
};

inline Palette darkPalette() { return { "#1f1f1f", "#2a2a2a", "#f2f2f2", "#9d9d9d", "#3a3a3a", "#333333", "#3d3d3d" }; }
inline Palette lightPalette() { return { "#ffffff", "#f3f3f3", "#1b1b1b", "#6b6b6b", "#e0e0e0", "#ebebeb", "#e0e0e0" }; }

inline QString buildStyle(const Palette& p, const QColor& accent, const QString& iconFont)
{
    return QString(R"(
QMainWindow, QSplitter, QStatusBar { background: %1; color: %3; }
QSplitter::handle { background: %5; width: 1px; }
QWidget#sidebar { background: %2; }
QLineEdit {
    background: %1; color: %3; border: 1px solid %5; border-radius: 6px;
    padding: 6px 10px; selection-background-color: %8;
}
QLineEdit:focus { border-bottom: 2px solid %8; }
QLineEdit#title {
    background: transparent; border: none; font-size: 24px; font-weight: 600;
    padding: 12px 20px 4px 20px;
}
QPlainTextEdit {
    background: %1; color: %3; border: none; padding: 4px 12px;
    selection-background-color: %8; selection-color: white;
}
QListWidget { background: transparent; border: none; outline: 0; }
QListWidget::item { color: %3; padding: 9px 10px; margin: 2px 0; border-radius: 6px; }
QListWidget::item:hover { background: %6; }
QListWidget::item:selected { background: %7; border-left: 3px solid %8; color: %3; }
QToolBar { background: %1; border: none; spacing: 4px; padding: 4px 8px; }
QToolBar::handle { background: %5; border-radius: 2px; margin: 6px 3px; }
QToolButton { background: transparent; color: %3; border: none; border-radius: 6px; padding: 6px 10px; }
QToolButton:hover { background: %6; }
QToolButton:pressed { background: %7; }
QToolBar QToolButton { font-family: "%9"; font-size: 16px; }
QLabel { color: %4; }
QScrollBar:vertical { background: transparent; width: 10px; margin: 0; }
QScrollBar::handle:vertical { background: %5; border-radius: 5px; min-height: 30px; }
QScrollBar::handle:vertical:hover { background: %4; }
QScrollBar::add-line:vertical, QScrollBar::sub-line:vertical { height: 0; }
QScrollBar:horizontal { background: transparent; height: 10px; }
QScrollBar::handle:horizontal { background: %5; border-radius: 5px; min-width: 30px; }
QScrollBar::add-line:horizontal, QScrollBar::sub-line:horizontal { width: 0; }
)")
.arg(p.bg, p.panel, p.text, p.sub, p.border, p.hover, p.selected, accent.name(), iconFont);
}