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

#include "svggenerator.h"

#include <QDebug>

namespace MongoChem {

SvgGenerator::SvgGenerator(QObject *parent_)
  : QObject(parent_)
{
}

SvgGenerator::~SvgGenerator()
{
}

void SvgGenerator::setInputData(const QByteArray &data)
{
  m_inputData = data;
}

QByteArray SvgGenerator::inputData() const
{
  return m_inputData;
}

void SvgGenerator::setInputFormat(const QByteArray &format)
{
  m_inputFormat = format;
}

QByteArray SvgGenerator::inputFormat() const
{
  return m_inputFormat;
}

QByteArray SvgGenerator::svg() const
{
  return m_svg;
}

void SvgGenerator::start()
{
  // set the options
  QStringList options;
  options << "-i" << m_inputFormat;
  options << "-osvg";
  options << "-xC"; // no terminal carbons lines

  // start the process and send the input data
  m_process.start("obabel", options);
  m_process.write(m_inputData);
  m_process.closeWriteChannel();

  // listen to the finished signal
  connect(&m_process, SIGNAL(finished(int, QProcess::ExitStatus)),
          this, SLOT(obabelFinished(int, QProcess::ExitStatus)));
}

void SvgGenerator::obabelFinished(int errorCode, QProcess::ExitStatus)
{
  // clear any previous data
  m_svg.clear();

  // get obabel output
  QByteArray output = m_process.readAllStandardOutput();

  // read and store svg data
  foreach (const QByteArray &line, output.split('\n')){
    if (line.startsWith("<rect"))
      continue;

    m_svg += line + '\n';
  }

  emit finished(errorCode);
}

} // end MongoChem namespace
