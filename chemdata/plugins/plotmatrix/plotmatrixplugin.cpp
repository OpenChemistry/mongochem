#include <chemdata/core/plugin.h>

#include "plotmatrixdialog.h"

class PlotMatrixPlugin : public ChemData::Plugin
{
public:
  PlotMatrixPlugin()
    : ChemData::Plugin("chemdata-plotmatrix")
  {
    CHEMDATA_REGISTER_ABSTRACT_VTK_CHART_WIDGET("plot-matrix", PlotMatrixDialog);
  }
};

CHEMDATA_EXPORT_PLUGIN(plotmatrix, PlotMatrixPlugin)
