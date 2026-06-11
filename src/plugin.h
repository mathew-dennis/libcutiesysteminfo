#pragma once
#include <QDebug>
#include <QtQuick>
#include <QtQml/qqml.h>
#include <QtQml/QQmlExtensionPlugin>

#include "cutiesysteminfo.h"

class CutieSystemInfoPlugin : public QQmlExtensionPlugin {
	Q_OBJECT
	Q_PLUGIN_METADATA(IID QQmlEngineExtensionInterface_iid FILE
			  "cutiesysteminfo.json")
    public:
	explicit CutieSystemInfoPlugin()
	{
	}

	void registerTypes(const char *uri) override;
};