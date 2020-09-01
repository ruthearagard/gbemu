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

// Required for the `QAction` class.
#include <qaction.h>

// Required for the `QCheckBox` class.
#include <qcheckbox.h>

// Required for the `QFormLayout` class.
#include <qformlayout.h>

// Required for the `QGridLayout` class.
#include <qgridlayout.h>

// Required for the `QGroupBox` class.
#include <qgroupbox.h>

// Required for the `QMenuBar` class.
#include <qmenubar.h>

// Required for the `QTreeWidget` class.
#include <qtreewidget.h>

// Required for the `CPUDebugger` class.
#include "cpu.h"

// Required for the `HexSpinBox` class.
#include "hexspinbox.h"

CPUDebugger::CPUDebugger(GameBoy::SystemBus& bus,
                         const GameBoy::CPU& cpu) noexcept : m_bus(bus),
                                                             m_cpu(cpu),
run{ .menu = menuBar()->addMenu("&Run"),
     .step = new QAction(tr("Step"), this) },
reg{ .group  = new QGroupBox(tr("Registers"), this),
     .layout = new QFormLayout(reg.group),
     .bc     = new HexSpinBox(this, 0xFFFF),
     .de     = new HexSpinBox(this, 0xFFFF),
     .hl     = new HexSpinBox(this, 0xFFFF),
     .af     = new HexSpinBox(this, 0xFFFF),
     .sp     = new HexSpinBox(this, 0xFFFF),
     .pc     = new HexSpinBox(this, 0xFFFF),
     .f      = { .group  = new QGroupBox(tr("Flag"), this),
                 .layout = new QVBoxLayout(reg.f.group),
                 .z      = new QCheckBox(tr("Z"), this),
                 .n      = new QCheckBox(tr("N"), this),
                 .h      = new QCheckBox(tr("H"), this),
                 .c      = new QCheckBox(tr("C"), this) }},
disassembly{ .group  = new QGroupBox(tr("Disassembly"), this),
             .layout = new QVBoxLayout(disassembly.group),
             .list   = new QTreeWidget(this) }
{
    setWindowTitle("gbemu CPU debugger");
    resize(800, 600);

    run.menu->addAction(run.step);

    main_widget = new QWidget(this);
    main_layout = new QGridLayout(main_widget);

    reg.layout->addRow("BC:", reg.bc);
    reg.layout->addRow("DE:", reg.de);
    reg.layout->addRow("HL:", reg.hl);
    reg.layout->addRow("AF:", reg.af);
    reg.layout->addRow("SP:", reg.sp);
    reg.layout->addRow("PC:", reg.pc);

    reg.f.layout->addWidget(reg.f.z);
    reg.f.layout->addWidget(reg.f.n);
    reg.f.layout->addWidget(reg.f.h);
    reg.f.layout->addWidget(reg.f.c);

    disassembly.list->setHeaderLabels(QStringList() << "PC" << "Instruction");

    disassembly.layout->addWidget(disassembly.list);

    main_layout->addWidget(reg.group, 0, 0);
    main_layout->addWidget(disassembly.group, 0, 1);
    main_layout->addWidget(reg.f.group, 1, 0);

    setCentralWidget(main_widget);
}

// Disables the interface if `condition` is true, or `false` otherwise.
auto CPUDebugger::disable(const bool condition) noexcept
{
    reg.bc->setDisabled(condition);
    reg.de->setDisabled(condition);
    reg.hl->setDisabled(condition);
    reg.af->setDisabled(condition);

    reg.pc->setDisabled(condition);
    reg.sp->setDisabled(condition);

    reg.f.z->setDisabled(condition);
    reg.f.n->setDisabled(condition);
    reg.f.h->setDisabled(condition);
    reg.f.c->setDisabled(condition);

    disassembly.list->setDisabled(condition);
}

void CPUDebugger::focusInEvent(QFocusEvent* e)
{
    disable(false);
}

void CPUDebugger::focusOutEvent(QFocusEvent* e)
{
    disable(true);
}