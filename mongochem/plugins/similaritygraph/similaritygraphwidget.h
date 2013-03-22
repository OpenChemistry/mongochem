/******************************************************************************

  This source file is part of the MongoChem project.

  Copyright 2011-2012 Kitware, Inc.

  This source code is released under the New BSD License, (the "License").

  Unless required by applicable law or agreed to in writing, software
  distributed under the License is distributed on an "AS IS" BASIS,
  WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
  See the License for the specific language governing permissions and
  limitations under the License.

******************************************************************************/

#ifndef SIMILARITYGRAPHWIDGET_H
#define SIMILARITYGRAPHWIDGET_H

#include <QWidget>

#include <vtkType.h>

#include <Eigen/Core>

class SimilarityGraphWidgetPrivate;

class SimilarityGraphWidget : public QWidget
{
  Q_OBJECT

public:
  /** Creates a new similarity graph widget with @p parent. */
  SimilarityGraphWidget(QWidget *parent = 0);

  /** Destroys the similarity graph widget. */
  ~SimilarityGraphWidget();

  /** Sets the similarity matrix. */
  void setSimilarityMatrix(const Eigen::MatrixXf &matrix);

  /** Returns the similarity matrix. */
  const Eigen::MatrixXf& similarityMatrix() const;

  /** Returns the similarity threshold. */
  float similarityThreshold() const;

  /** Pauses/unpaused the graph layout algorithm. */
  void setLayoutPaused(bool paused = true);

  /** Returns @c true if the graph layout algorithm is paused. */
  bool isLayoutPaused() const;

public slots:
  /**
   * Sets the similarity threshold to @p value. The value should be between 0.0
   * and 1.0.
   */
  void setSimilarityThreshold(float value);

signals:
  void readyToRender();

  /** This signal is emitted when a graph vertex is double clicked. */
  void vertexDoubleClicked(vtkIdType id);

private slots:
  void updateLayout();
  void renderGraph();
  void graphViewMouseEvent(QMouseEvent *event);

private:
  SimilarityGraphWidgetPrivate* const d;
};

#endif // SIMILARITYGRAPHWIDGET_H
