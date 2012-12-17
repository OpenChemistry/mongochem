
#include "abstractimportdialog.h"

#include <chemkit/pluginmanager.h>

namespace ChemData {

AbstractImportDialog::AbstractImportDialog(QWidget *parent_)
  : QDialog(parent_)
{
}

AbstractImportDialog::~AbstractImportDialog()
{
}

std::vector<std::string> AbstractImportDialog::importers()
{
  chemkit::PluginManager *pluginManager = chemkit::PluginManager::instance();

  return pluginManager->pluginClassNames<AbstractImportDialog>();
}

AbstractImportDialog* AbstractImportDialog::create(const std::string &name)
{
  chemkit::PluginManager *pluginManager = chemkit::PluginManager::instance();

  return pluginManager->createPluginClass<AbstractImportDialog>(name);
}

} // end ChemData namespace
