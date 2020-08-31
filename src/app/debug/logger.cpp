// Copyright 2020 Michael Rodriguez
//
// Permission to use, copy, modify, and/or distribute this software for any
// purpose with or without fee is hereby granted, provided that the above
// copyright notice and this permission notice appear in all copies.
//
// THE SOFTWARE IS PROVIDED "AS IS" AND THE AUTHOR DISCLAIMS ALL WARRANTIES
// WITH REGARD TO THIS SOFTWARE INCLUDING ALL IMPLIED WARRANTIES OF
// MERCHANTABILITY AND FITNESS.IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR ANY
// SPECIAL, DIRECT, INDIRECT, OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES
// WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS, WHETHER IN AN ACTION
// OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING OUT OF OR IN
// CONNECTION WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.

// Required for the `QFileDialog` class.
#include <qfiledialog.h>

// Required for the `QFontDialog` class.
#include <qfontdialog.h>

// Required for the `QMenuBar` class.
#include <qmenubar.h>

// Required for the `QPlainTextEdit` class.
#include <qplaintextedit.h>

// Required for the `QSettings` class.
#include <qsettings.h>

// Required for the `QTextStream` class.
#include <qtextstream.h>

// Required for the `MessageLogger` class.
#include "logger.h"

MessageLogger::MessageLogger(QWidget* parent) : QMainWindow(parent)
{
    setWindowTitle("gbemu logger");
    resize(680, 480);

    file.menu = menuBar()->addMenu(tr("&File"));

    file.save = new QAction(tr("&Save..."), this);

    connect(file.save,
        &QAction::triggered,
        this,
        &MessageLogger::on_save_log);

    file.menu->addAction(file.save);

    view.menu = menuBar()->addMenu(tr("&View"));

    view.select_font = new QAction(tr("&Font..."), this);
    view.clear_log   = new QAction(tr("&Clear"),   this);

    connect(view.select_font,
            &QAction::triggered,
            this,
            &MessageLogger::on_select_font);

    connect(view.clear_log, &QAction::triggered, this, &MessageLogger::reset);

    view.menu->addAction(view.select_font);
    view.menu->addAction(view.clear_log);

    text_edit = new QPlainTextEdit(this);
    text_edit->setReadOnly(true);
    text_edit->setStyleSheet("background-color: black");

    QSettings config_file("gbtest.ini", QSettings::IniFormat, this);

    const auto font_name
    {
        config_file.value("message_logger/font_name",
                          "Lucida Console").toString()
    };

    const auto font_size
    {
        config_file.value("message_logger/font_size", 10).toInt()
    };

    QFont font(font_name, font_size);
    font.setBold(true);

    auto doc{ text_edit->document() };
    doc->setDefaultFont(font);

    QPalette p = text_edit->palette();
    p.setColor(QPalette::Base, Qt::red);
    p.setColor(QPalette::Text, Qt::red);
    text_edit->setPalette(p);

    setCentralWidget(text_edit);
}

auto MessageLogger::append(const QString& data) -> void
{
    text_edit->insertPlainText(data);
}

auto MessageLogger::reset() -> void
{
    text_edit->clear();
}

auto MessageLogger::on_select_font() -> void
{
    bool ok;

    auto font
    {
        QFontDialog::getFont(&ok,
                             QFont("Lucida Console", 10),
                             this)
    };

    if (ok)
    {
        auto doc{ text_edit->document() };
        doc->setDefaultFont(font);

        QSettings config_file("gbtest.ini", QSettings::IniFormat, this);

        config_file.setValue("message_logger/font_name", font.toString());
        config_file.setValue("message_logger/font_size", font.pointSize());
    }
}

// Called when the user triggers "File -> Save" to save the log contents.
auto MessageLogger::on_save_log() -> void
{
    const auto file_name
    {
        QFileDialog::getSaveFileName(this,
                                     tr("Save log"),
                                     "",
                                     tr("Log files (*.txt)"))
    };

    if (!file_name.isEmpty())
    {
        QFile log_file(file_name);

        log_file.open(QIODevice::WriteOnly | QIODevice::Text);

        QTextStream out(&log_file);
        out << text_edit->toPlainText();

        log_file.close();
    }
}