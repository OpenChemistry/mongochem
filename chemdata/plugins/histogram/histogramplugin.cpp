#include <chemdata/core/plugin.h>

#include "histogramdialog.h"

class HistogramPlugin : public ChemData::Plugin
{
public:
  HistogramPlugin()
    : ChemData::Plugin("chemdata-histogram")
  {
    CHEMDATA_REGISTER_ABSTRACT_VTK_CHART_WIDGET("histogram", HistogramDialog);
  }
};

CHEMDATA_EXPORT_PLUGIN(histogram, HistogramPlugin)
