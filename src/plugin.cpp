#include "plugin.h"
#include "cutiesysteminfo.h"

void CutieSystemInfoPlugin::registerTypes(const char *uri)
{
    qmlRegisterType<CutieSystemInfo>(uri, 1, 0, "CutieSystemInfo");
}