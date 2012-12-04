
#include "abstractvtkchartwidget.h"

#include <chemkit/pluginmanager.h>

namespace ChemData {

AbstractVtkChartWidget::AbstractVtkChartWidget(QWidget *parent_)
  : QWidget(parent_)
{
  m_selectionLink = 0;
}

AbstractVtkChartWidget::~AbstractVtkChartWidget()
{
}

void AbstractVtkChartWidget::setSelectionLink(vtkAnnotationLink *link)
{
  m_selectionLink = link;
}

vtkAnnotationLink* AbstractVtkChartWidget::selectionLink() const
{
  return m_selectionLink;
}

std::vector<std::string> AbstractVtkChartWidget::widgets()
{
  chemkit::PluginManager *pluginManager = chemkit::PluginManager::instance();

  return pluginManager->pluginClassNames<AbstractVtkChartWidget>();
}

AbstractVtkChartWidget* AbstractVtkChartWidget::create(const std::string &name)
{
  chemkit::PluginManager *pluginManager = chemkit::PluginManager::instance();

  return pluginManager->createPluginClass<AbstractVtkChartWidget>(name);
}

} // end ChemData namespace
