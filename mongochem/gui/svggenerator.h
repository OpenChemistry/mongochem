/******************************************************************************

  This source file is part of the MongoChem project.

  Copyright 2013 Kitware, Inc.

  This source code is released under the New BSD License, (the "License").

  Unless required by applicable law or agreed to in writing, software
  distributed under the License is distributed on an "AS IS" BASIS,
  WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
  See the License for the specific language governing permissions and
  limitations under the License.

******************************************************************************/

#ifndef SVGGENERATOR_H
#define SVGGENERATOR_H

#include <QObject>
#include <QProcess>

namespace MongoChem {

/**
 * The SvgGenerator class generates SVG diagrams for molecules.
 */
class SvgGenerator : public QObject
{
  Q_OBJECT

public:
  /**
   * Creates a new SVG generator object.
   */
  SvgGenerator(QObject *parent_ = 0);

  /**
   * Destroys the SVG generator object.
   */
  ~SvgGenerator();

  /**
   * Sets the input data.
   */
  void setInputData(const QByteArray &data);

  /**
   * Returns the input data.
   */
  QByteArray inputData() const;

  /**
   * Sets the input format.
   */
  void setInputFormat(const QByteArray &format);

  /**
   * Returns the input format.
   */
  QByteArray inputFormat() const;

  /**
   * Returns the generated SVG.
   */
  QByteArray svg() const;

  /**
   * Starts the generation process. When finished, the finished() signal will
   * be emitted.
   */
  void start();

signals:
  /**
   * This signal is emitted when the generation process is complete. The
   * errorCode will be 0 if the generation was successful.
   */
  void finished(int errorCode);

private slots:
  void obabelFinished(int errorCode, QProcess::ExitStatus exitStatus);

private:
  Q_DISABLE_COPY(SvgGenerator)

  QProcess m_process;
  QByteArray m_inputData;
  QByteArray m_inputFormat;
  QByteArray m_svg;
};

} // end MongoChem namespace

#endif // SVGGENERATOR_H
