#include <chemdata/core/plugin.h>

#include "parallelcoordinatesdialog.h"

class ParallelCoordinatesPlugin : public ChemData::Plugin
{
public:
  ParallelCoordinatesPlugin()
    : ChemData::Plugin("chemdata-parallelcoordinates")
  {
    CHEMDATA_REGISTER_ABSTRACT_VTK_CHART_WIDGET("parallel-coordinates", ParallelCoordinatesDialog);
  }
};

CHEMDATA_EXPORT_PLUGIN(parallelcoordinates, ParallelCoordinatesPlugin)
