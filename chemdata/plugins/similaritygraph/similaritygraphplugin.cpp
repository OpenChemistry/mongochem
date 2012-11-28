#include <chemkit/plugin.h>

#include "fingerprintsimilaritydialog.h"
#include "structuresimilaritydialog.h"

class SimilarityGraphPlugin : public ChemData::Plugin
{
public:
  SimilarityGraphPlugin()
    : ChemData::Plugin("chemdata-similaritygraph")
  {
    CHEMDATA_REGISTER_ABSTRACT_CLUSTERING_WIDGET("fingerprint-similarity", FingerprintSimilarityDialog);
    CHEMDATA_REGISTER_ABSTRACT_CLUSTERING_WIDGET("structure-similarity", StructureSimilarityDialog);
  }
};

CHEMDATA_EXPORT_PLUGIN(similaritygraph, SimilarityGraphPlugin)
