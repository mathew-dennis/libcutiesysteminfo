#include "plugin.h"
#include "cutiesysteminfo.h"

void CutieSystemInfoPlugin::registerTypes(const char *uri)
{
    // 1. Tell QML about the sub-objects so it can read their properties
    qmlRegisterUncreatableType<CutieOSInfo>(uri, 1, 0, "CutieOSInfo", 
        "CutieOSInfo is internal and cannot be instantiated directly in QML.");
        
    qmlRegisterUncreatableType<CutieHardwareInfo>(uri, 1, 0, "CutieHardwareInfo", 
        "CutieHardwareInfo is internal and cannot be instantiated directly in QML.");

    // 2. Register your main instantiable class
    qmlRegisterType<CutieSystemInfo>(uri, 1, 0, "CutieSystemInfo");
}
