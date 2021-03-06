/*
Copyright (C) 2017 Ruslan Kabatsayev <b7.10110111@gmail.com>

This program is free software: you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation, either version 2 of the License, or
(at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program.  If not, see <http://www.gnu.org/licenses/>.
*/

#include "Plugin.h"
#include "edb.h"
#include <QMenu>
#include <QDebug>
#include <QMainWindow>
#include <QDockWidget>
#include <QPlainTextEdit>
#include <iostream>

namespace DebuggerErrorConsolePlugin
{

Plugin* Plugin::instance=nullptr;

#if QT_VERSION < 0x50000
void Plugin::debugMessageIntercept(QtMsgType type, const char* message_)
{
	const QString message(message_);
#else
void Plugin::debugMessageIntercept(QtMsgType type, QMessageLogContext const&, QString const& message)
{
#endif
	if(!instance) return;

	QString text;
	switch(type)
	{
	case QtDebugMsg:
		text+="DEBUG";
		break;
#if QT_VERSION>=0x050500
	case QtInfoMsg:
		text+="INFO ";
		break;
#endif
	case QtWarningMsg:
		text+="WARN ";
		break;
	case QtCriticalMsg:
		text+="ERROR";
		break;
	case QtFatalMsg:
		text+="FATAL";
		break;
	}
	text+="  "+QString(message);
	instance->textWidget_->appendPlainText(text);
	std::cerr << message.toUtf8().constData() << "\n"; // this may be useful as a crash log
}

Plugin::Plugin()
{
	instance=this;

	textWidget_ = new QPlainTextEdit;
	textWidget_->setReadOnly(true);
	QFont font("monospace");
	font.setStyleHint(QFont::TypeWriter);
	textWidget_->setFont(font);

#if QT_VERSION < 0x50000
	qInstallMsgHandler(debugMessageIntercept);
#else
	qInstallMessageHandler(debugMessageIntercept);
#endif
}

Plugin::~Plugin()
{
	instance=nullptr;
}

QMenu* Plugin::menu(QWidget* parent)
{
	if(!menu_)
	{
		if(auto*const mainWindow = qobject_cast<QMainWindow *>(edb::v1::debugger_ui))
		{
			auto*const dockWidget = new QDockWidget(tr("Debugger Error Console"), mainWindow);
			dockWidget->setObjectName(QString::fromUtf8("Debugger Error Console"));
			dockWidget->setWidget(textWidget_);

			mainWindow->addDockWidget(Qt::BottomDockWidgetArea, dockWidget);

			menu_ = new QMenu(tr("Debugger Error Console"), parent);
			menu_->addAction(dockWidget->toggleViewAction());

			auto docks = mainWindow->findChildren<QDockWidget *>();
			// We want to put it to the same area as Stack dock
			// Stupid QList doesn't have a reverse iterator
			for(auto it=docks.end()-1;;--it)
			{
				QDockWidget*const widget=*it;
				if(widget!=dockWidget && mainWindow->dockWidgetArea(widget)==Qt::BottomDockWidgetArea)
				{
					mainWindow->tabifyDockWidget(widget, dockWidget);

					widget->show();
					widget->raise();
					break;
				}
				if(it==docks.begin())
					break;
			}

		}
	}
	return menu_;
}


DebuggerErrorConsole::DebuggerErrorConsole(QWidget* parent)
	: QDialog(parent)
{
}

#if QT_VERSION < 0x050000
Q_EXPORT_PLUGIN2(Plugin, Plugin)
#endif

}
