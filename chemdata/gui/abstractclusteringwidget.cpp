
#include "abstractclusteringwidget.h"

#include <chemkit/pluginmanager.h>

namespace ChemData {

AbstractClusteringWidget::AbstractClusteringWidget(QWidget *parent_)
  : QWidget(parent_)
{
}

AbstractClusteringWidget::~AbstractClusteringWidget()
{
}

void AbstractClusteringWidget::setMolecules(const std::vector<MoleculeRef> &molecules)
{
  Q_UNUSED(molecules);
}

std::vector<std::string> AbstractClusteringWidget::widgets()
{
  chemkit::PluginManager *pluginManager = chemkit::PluginManager::instance();

  return pluginManager->pluginClassNames<AbstractClusteringWidget>();
}

AbstractClusteringWidget* AbstractClusteringWidget::create(const std::string &name)
{
  chemkit::PluginManager *pluginManager = chemkit::PluginManager::instance();

  return pluginManager->createPluginClass<AbstractClusteringWidget>(name);
}

} // end ChemData namespace
