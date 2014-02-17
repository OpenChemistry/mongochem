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

#ifndef MONGOCHEMGUI_ABSTRACTVTKCHARTWIDGET_H
#define MONGOCHEMGUI_ABSTRACTVTKCHARTWIDGET_H

#include "mongochemguiexport.h"

#include <QtWidgets/QWidget>

class vtkAnnotationLink;

namespace MongoChem {

/**
 * @class AbstractVtkChartWidget
 *
 * The AbstractVtkChartWidget class is an abstract base-class for
 * VTK-based chart widgets.
 */
class MONGOCHEMGUI_EXPORT AbstractVtkChartWidget : public QWidget
{
  Q_OBJECT

public:
  ~AbstractVtkChartWidget();

  /** Sets the selection link for the chart widget. */
  virtual void setSelectionLink(vtkAnnotationLink *link);

  /** Returns the selection link for the chart widget. */
  vtkAnnotationLink* selectionLink() const;

  /** Creates a new chart widget. */
  explicit AbstractVtkChartWidget(QWidget *parent_ = 0);

private:
  vtkAnnotationLink *m_selectionLink;
};

/**
 * @class AbstractVtkChartWidgetFactory
 * @brief The base class for chart widget factories in MongoChem.
 * @author Marcus D. Hanwell
 */
class MONGOCHEMGUI_EXPORT AbstractVtkChartWidgetFactory
{
public:
  virtual ~AbstractVtkChartWidgetFactory() {}

  virtual AbstractVtkChartWidget * createInstance() = 0;
  virtual QString identifier() const = 0;
};

} // end MongoChem namespace

Q_DECLARE_INTERFACE(MongoChem::AbstractVtkChartWidgetFactory,
                    "org.openchemistry.mongochem.ChartWidgetFactory")

#endif // MONGOCHEMGUI_ABSTRACTVTKCHARTWIDGET_H
