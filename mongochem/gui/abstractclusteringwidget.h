/******************************************************************************

  This source file is part of the MongoChem project.

  Copyright 2012 Kitware, Inc.

  This source code is released under the New BSD License, (the "License").

  Unless required by applicable law or agreed to in writing, software
  distributed under the License is distributed on an "AS IS" BASIS,
  WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
  See the License for the specific language governing permissions and
  limitations under the License.

******************************************************************************/

#ifndef MONGOCHEMGUI_ABSTRACTCLUSTERINGWIDGET_H
#define MONGOCHEMGUI_ABSTRACTCLUSTERINGWIDGET_H

#include "mongochemguiexport.h"

#include "moleculeref.h"

#include <QtGui/QWidget>

namespace MongoChem {

/**
 * @class AbstractVtkChartWidget
 *
 * The AbstractClusteringWidget class is an abstract base-class for
 * clustering visualization widgets.
 */
class MONGOCHEMGUI_EXPORT AbstractClusteringWidget : public QWidget
{
  Q_OBJECT

public:
  /** Sets the molecules to display. */
  virtual void setMolecules(const std::vector<MoleculeRef> &molecules);

signals:
  void moleculeDoubleClicked(MongoChem::MoleculeRef ref);

protected:
  /** Creates a new clustering widget. */
  AbstractClusteringWidget(QWidget *parent_ = 0);
};

/**
 * @class AbstractClusteringWidgetFactory
 * @brief The base class for clustering widget factories in MongoChem.
 * @author Marcus D. Hanwell
 */
class MONGOCHEMGUI_EXPORT AbstractClusteringWidgetFactory
{
public:
  virtual ~AbstractClusteringWidgetFactory() {}

  virtual AbstractClusteringWidget * createInstance() = 0;
  virtual QString identifier() const = 0;
};

} // end MongoChem namespace

Q_DECLARE_INTERFACE(MongoChem::AbstractClusteringWidgetFactory,
                    "org.openchemistry.mongochem.clusteringwidgetfactory/0.1")


#endif // MONGOCHEMGUI_ABSTRACTCLUSTERINGWIDGET_H
