#pragma once
#include "stdafx.h"
#include "Settings.h"

class SettingsDialog : public QDialog
{
public:
    SettingsDialog(const AppSettings& in, QWidget* parent = nullptr)
        : QDialog(parent), result(in)
    {
        setWindowTitle("Ustawienia");
        setMinimumWidth(380);

        auto* form = new QFormLayout;

        themeBox = new QComboBox;
        themeBox->addItems({ "Systemowy", "Jasny", "Ciemny" });
        themeBox->setCurrentIndex(static_cast<int>(in.theme));
        form->addRow("Motyw", themeBox);

        accentBtn = new QPushButton;
        updateAccentButton();
        connect(accentBtn, &QPushButton::clicked, this, [this] {
            QColor c = QColorDialog::getColor(result.accent, this, "Kolor akcentu");
            if (c.isValid()) { result.accent = c; updateAccentButton(); }
            });
        form->addRow("Kolor akcentu", accentBtn);

        fontBox = new QFontComboBox;
        fontBox->setFontFilters(QFontComboBox::MonospacedFonts);
        fontBox->setCurrentFont(QFont(in.fontFamily));
        form->addRow("Czcionka", fontBox);

        sizeBox = new QSpinBox;
        sizeBox->setRange(8, 48);
        sizeBox->setValue(in.fontSize);
        form->addRow("Rozmiar", sizeBox);

        wrapBox = new QCheckBox("Zawijanie wierszy");
        wrapBox->setChecked(in.wordWrap);
        form->addRow(wrapBox);

        statusBox = new QCheckBox("Pasek stanu");
        statusBox->setChecked(in.showStatusBar);
        form->addRow(statusBox);

        saveBox = new QSpinBox;
        saveBox->setRange(200, 10000);
        saveBox->setSingleStep(100);
        saveBox->setSuffix(" ms");
        saveBox->setValue(in.autosaveMs);
        form->addRow("Opóźnienie autozapisu", saveBox);

        auto* buttons = new QDialogButtonBox(QDialogButtonBox::Ok | QDialogButtonBox::Cancel);
        connect(buttons, &QDialogButtonBox::accepted, this, &QDialog::accept);
        connect(buttons, &QDialogButtonBox::rejected, this, &QDialog::reject);

        auto* root = new QVBoxLayout(this);
        root->setContentsMargins(20, 20, 20, 20);
        root->addLayout(form);
        root->addWidget(buttons);
    }

    AppSettings value() const
    {
        AppSettings s = result;
        s.theme = static_cast<ThemeMode>(themeBox->currentIndex());
        s.fontFamily = fontBox->currentFont().family();
        s.fontSize = sizeBox->value();
        s.wordWrap = wrapBox->isChecked();
        s.showStatusBar = statusBox->isChecked();
        s.autosaveMs = saveBox->value();
        return s;
    }

private:
    void updateAccentButton()
    {
        accentBtn->setText(result.accent.name().toUpper());
        accentBtn->setStyleSheet(QString("QPushButton { background: %1; color: white; border: none; "
            "border-radius: 6px; padding: 6px 12px; }")
            .arg(result.accent.name()));
    }

    AppSettings result;
    QComboBox* themeBox; QPushButton* accentBtn; QFontComboBox* fontBox;
    QSpinBox* sizeBox; QCheckBox* wrapBox; QCheckBox* statusBox; QSpinBox* saveBox;
};