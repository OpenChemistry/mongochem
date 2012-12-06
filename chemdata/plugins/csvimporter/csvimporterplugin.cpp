#include <chemdata/core/plugin.h>

#include "importcsvfiledialog.h"

class CsvImporterPlugin : public ChemData::Plugin
{
public:
  CsvImporterPlugin()
    : ChemData::Plugin("chemdata-csv-importer")
  {
    CHEMDATA_REGISTER_ABSTRACT_IMPORT_DIALOG("csv", ImportCsvFileDialog);
  }
};

CHEMDATA_EXPORT_PLUGIN(csvimporter, CsvImporterPlugin)
