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

#include <string>
#include <sstream>

#include <QBuffer>
#include <QWebView>

namespace MongoChem {

struct OBabelJob
{
    QProcess *process;
    QStringList options;
    QByteArray input;
};

QQueue<OBabelJob> SvgGenerator::m_jobs;
int SvgGenerator::m_runningJobs = 0;

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

QByteArray SvgGenerator::png() const
{
  QByteArray png_;

  if (m_svg.isEmpty())
    return png_;

  QImage image_ = image();
  QBuffer buffer(&png_);
  buffer.open(QIODevice::WriteOnly);
  image_.save(&buffer, "PNG");
  buffer.close();

  return png_;
}

QImage SvgGenerator::image() const
{
  QImage img(250, 250, QImage::Format_ARGB32_Premultiplied);
  if (m_svg.isEmpty())
    return img;

  // create web view
  QWebView widget;

  // set transparent background
  QPalette palette_ = widget.palette();
  palette_.setBrush(QPalette::Base, Qt::white);
  widget.page()->setPalette(palette_);

  // set content
  std::stringstream html;
  html << "<html>"
       << "<head>"
       << "<style type=\"text/css\">"
       << "html, body { margin:0; padding:0; overflow:hidden }"
       << "svg { top:0; left:0; height:100%; width:100% }"
       << "</style>"
       << "</head>"
       << "<body>"
       << m_svg.constData()
       << "</body>"
       << "</html>";
  std::string content = html.str();
  widget.setContent(QByteArray(content.c_str()));

  // render to image
  widget.render(&img);

  return img;
}

void SvgGenerator::start()
{
  // set the options
  QStringList options;
  options << "-i" << m_inputFormat;
  options << "-osvg";
  options << "-xC"; // no terminal carbons lines

  // listen to the finished signal
  connect(&m_process, SIGNAL(finished(int, QProcess::ExitStatus)),
          this, SLOT(obabelFinished(int, QProcess::ExitStatus)));

  OBabelJob job;
  job.process = &m_process;
  job.options = options;
  job.input = m_inputData;

  if (m_runningJobs < 5)
    runJob(job);
  else
    m_jobs.enqueue(job);
}

void SvgGenerator::obabelFinished(int errorCode, QProcess::ExitStatus)
{
  m_runningJobs--;

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

  // start next job in the queue
  if (!m_jobs.empty() && m_runningJobs < 5) {
    runJob(m_jobs.dequeue());
  }
}

void SvgGenerator::runJob(const OBabelJob &job)
{
  QProcess *process = const_cast<QProcess *>(job.process);

  // start the process and send the input data
  process->start("obabel", job.options);
  process->write(job.input);
  process->closeWriteChannel();

  m_runningJobs++;
}

} // end MongoChem namespace
