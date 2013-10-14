/***************************************************************************
 *   Copyright (C) 2010 by P. Sereno                                       *
 *   http://www.sereno-online.com                                          *
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU Lesser General Public License           *
 *   version 2.1 as published by the Free Software Foundation              *
 *                                                                         *
 *   This program is distributed in the hope that it will be useful,       *
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of        *
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the         *
 *   GNU Lesser General Public License for more details.                   *
 *   http://www.gnu.org/licenses/old-licenses/lgpl-2.1.html.               *
 ***************************************************************************/

#include "qled.h"
#include "qledplugin.h"
#include <QtPlugin>

// See Cusotm Widget Plugin Example in Qt online documentation to understand the meaning of this file

QLedPlugin::QLedPlugin(QObject *parent)
    : QObject(parent)
{
    initialized = false;
}

void QLedPlugin::initialize(QDesignerFormEditorInterface * /* core */)
{
    if (initialized)
        return;

    initialized = true;
}

bool QLedPlugin::isInitialized() const
{
    return initialized;
}

QWidget *QLedPlugin::createWidget(QWidget *parent)
{
    return new QLed(parent);
}

QString QLedPlugin::name() const
{
    return "QLed";
}

QString QLedPlugin::group() const
{
    return "Lab Widgets";  //change this if you want to put the custom widget plugin in another group
}

QIcon QLedPlugin::icon() const
{
    return QIcon(":resources/qled.png");
}

QString QLedPlugin::toolTip() const
{
    return tr("Led Custom widget Plugin fot Qt Designer");
}

QString QLedPlugin::whatsThis() const
{
    return tr("Led Custom widget Plugin fot Qt Designer");
}

bool QLedPlugin::isContainer() const
{
    return false;
}

QString QLedPlugin::domXml() const
{
    return "<widget class=\"QLed\" name=\"qLed\">\n"
           " <property name=\"geometry\">\n"
           "  <rect>\n"
           "   <x>0</x>\n"
           "   <y>0</y>\n"
           "   <width>50</width>\n"
           "   <height>50</height>\n"
           "  </rect>\n"
           " </property>\n"
           " <property name=\"toolTip\" >\n"
           "  <string>Binary Led</string>\n"
           " </property>\n"
           " <property name=\"value\" >\n"
           " <bool>false</bool>\n"
           " </property>\n"
           " <property name=\"whatsThis\" >\n"
           "  <string>Led widget</string>\n"
           " </property>\n"
           " <property name=\"onColor\" >\n"
           " <enum>QLed::Red</enum>\n"
           " </property>\n"
           " <property name=\"offColor\" >\n"
           " <enum>QLed::Grey</enum>\n"
           " </property>\n"
           " <property name=\"shape\" >\n"
           " <enum>QLed::Circle</enum>\n"
           " </property>\n"
           "</widget>\n";
}

QString QLedPlugin::includeFile() const
{
    return "qled.h";
}

Q_EXPORT_PLUGIN2(customwidgetplugin, QLedPlugin)
