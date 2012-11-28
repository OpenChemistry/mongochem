#ifndef CHEMDATA_GUI_ABSTRACTVTKCHARTWIDGET_H
#define CHEMDATA_GUI_ABSTRACTVTKCHARTWIDGET_H

#include <chemdata/gui/export.h>
#include <chemdata/core/chemdata.h>
#include <chemdata/core/plugin.h>

#include <QWidget>

class vtkAnnotationLink;

namespace ChemData {

/// \class AbstractVtkChartWidget
///
/// The AbstractVtkChartWidget class is an abstract base-class for
/// VTK-based chart widgets.
class CHEMDATAGUI_EXPORT AbstractVtkChartWidget : public QWidget
{
  Q_OBJECT

public:
  /// Destroys the chart widget.
  ~AbstractVtkChartWidget() CHEMDATA_OVERRIDE;

  /// Sets the selection link for the chart widget.
  virtual void setSelectionLink(vtkAnnotationLink *link);

  /// Returns the selection link for the chart widget.
  vtkAnnotationLink* selectionLink() const;

  /// Returns a list of names of all registered chart widgets.
  static std::vector<std::string> widgets();

  /// Creates and returns a new chart widget with \p name. The ownership of
  /// the returned pointer is passed to the caller.
  static AbstractVtkChartWidget* create(const std::string &name);

protected:
  /// Creates a new chart widget.
  explicit AbstractVtkChartWidget(QWidget *parent_ = 0);

private:
  vtkAnnotationLink *m_selectionLink;
};

} // end ChemData namespace

#define CHEMDATA_REGISTER_ABSTRACT_VTK_CHART_WIDGET(name, className) \
  CHEMDATA_REGISTER_PLUGIN_CLASS(name, ChemData::AbstractVtkChartWidget, className)

#endif // CHEMDATA_GUI_ABSTRACTVTKCHARTWIDGET_H
