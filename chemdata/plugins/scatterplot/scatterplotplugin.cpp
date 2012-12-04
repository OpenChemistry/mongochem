#include <chemkit/plugin.h>

#include "scatterplotdialog.h"

class ScatterPlotPlugin : public ChemData::Plugin
{
public:
  ScatterPlotPlugin()
    : ChemData::Plugin("chemdata-scatterplot")
  {
    CHEMDATA_REGISTER_ABSTRACT_VTK_CHART_WIDGET("scatter-plot", ScatterPlotDialog);
  }
};

CHEMDATA_EXPORT_PLUGIN(scatterplot, ScatterPlotPlugin)
