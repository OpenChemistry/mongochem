#include <chemdata/core/plugin.h>

#include "kmeansclusteringdialog.h"

class KMeansClusteringPlugin : public ChemData::Plugin
{
public:
  KMeansClusteringPlugin()
    : ChemData::Plugin("chemdata-kmeansclustering")
  {
    CHEMDATA_REGISTER_ABSTRACT_CLUSTERING_WIDGET("kmeans-clustering", KMeansClusteringDialog);
  }
};

CHEMDATA_EXPORT_PLUGIN(kmeansclustering, KMeansClusteringPlugin)
