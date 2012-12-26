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

#include "abstractvtkchartwidget.h"

namespace MongoChem {

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

} // end MongoChem namespace
