#ifndef CHEMDATA_ABSTRACTIMPORTDIALOG_H
#define CHEMDATA_ABSTRACTIMPORTDIALOG_H

#include <string>
#include <vector>

#include <chemdata/gui/export.h>
#include <chemdata/core/chemdata.h>
#include <chemdata/core/plugin.h>

#include <QDialog>

namespace ChemData {

/// \class AbstractImportDialog
///
/// The AbstractImportDialog class is an abstract base-class for
/// data import dialogs.
class CHEMDATAGUI_EXPORT AbstractImportDialog : public QDialog
{
  Q_OBJECT

public:
  /// Destroys the import dialog.
  ~AbstractImportDialog();

  /// Returns a list of names of all registered importers.
  static std::vector<std::string> importers();

  /// Creates and returns a new import dialog with \p name. The ownership
  /// of the returned pointer is passed to the caller.
  static AbstractImportDialog* create(const std::string &name);

protected:
  /// Creates a new import dialog.
  explicit AbstractImportDialog(QWidget *parent_ = 0);
};

} // end ChemData namespace

#define CHEMDATA_REGISTER_ABSTRACT_IMPORT_DIALOG(name, className) \
  CHEMDATA_REGISTER_PLUGIN_CLASS(name, ChemData::AbstractImportDialog, className)

#endif // CHEMDATA_ABSTRACTIMPORTDIALOG_H
