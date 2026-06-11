#include "plugin.h"

void CutieSystemInfoPlugin::registerTypes(const char *uri)
{
	qmlRegisterSingletonType<CutieSystemInfo>(uri, 1, 0, "CutieSystemInfo",
					      &CutieSystemInfo::provider);
}
