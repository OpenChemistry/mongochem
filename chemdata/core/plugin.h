#ifndef CHEMDATA_CORE_PLUGIN_H
#define CHEMDATA_CORE_PLUGIN_H

#include <chemkit/plugin.h>

namespace ChemData {

typedef chemkit::Plugin Plugin;

} // end ChemData namespace

#define CHEMDATA_EXPORT_PLUGIN CHEMKIT_EXPORT_PLUGIN
#define CHEMDATA_REGISTER_PLUGIN_CLASS CHEMKIT_REGISTER_PLUGIN_CLASS

#endif // CHEMDATA_CORE_PLUGIN_H
