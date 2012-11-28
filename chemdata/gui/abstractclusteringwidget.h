#ifndef CHEMDATA_GUI_ABSTRACTCLUSTERINGWIDGET_H
#define CHEMDATA_GUI_ABSTRACTCLUSTERINGWIDGET_H

#include <chemdata/gui/export.h>
#include <chemdata/core/chemdata.h>
#include <chemdata/core/plugin.h>
#include <chemdata/core/moleculeref.h>

#include <QWidget>

namespace ChemData {

/// \class AbstractVtkChartWidget
///
/// The AbstractClusteringWidget class is an abstract base-class for
/// clustering visualization widgets.
class CHEMDATAGUI_EXPORT AbstractClusteringWidget : public QWidget
{
  Q_OBJECT

public:
  /// Destroys the clustering widget.
  ~AbstractClusteringWidget() CHEMDATA_OVERRIDE;

  /// Sets the molecules to display.
  virtual void setMolecules(const std::vector<MoleculeRef> &molecules);

    /// Returns a list of names of all registered clustering widgets.
  static std::vector<std::string> widgets();

  /// Creates and returns a new clustering widget with \p name. The ownership
  /// of the returned pointer is passed to the caller.
  static AbstractClusteringWidget* create(const std::string &name);

signals:
  void moleculeDoubleClicked(ChemData::MoleculeRef ref);

protected:
  /// Creates a new clustering widget.
  AbstractClusteringWidget(QWidget *parent_ = 0);
};

} // end ChemData namespace

#define CHEMDATA_REGISTER_ABSTRACT_CLUSTERING_WIDGET(name, className) \
  CHEMDATA_REGISTER_PLUGIN_CLASS(name, ChemData::AbstractClusteringWidget, className)

#endif // CHEMDATA_GUI_ABSTRACTCLUSTERINGWIDGET_H
