mongochem_plugin(FingerprintSimilarity
  "Fingerprint Similarity"
  AbstractClusteringWidget
  fingerprintsimilaritydialog.h
  FingerprintSimilarityDialog
  "fingerprintsimilaritydialog.cpp;similaritygraphwidget.cpp"
  fingerprintsimilaritydialog.ui
)
target_link_libraries(FingerprintSimilarity
  vtkChartsCore vtkGUISupportQt vtkRenderingQt vtkViewsContext2D vtkViewsInfovis
)

#mongochem_plugin(StructureSimilarity
#  "Structure Similarity"
#  AbstractClusteringWidget
#  structuresimilaritydialog.h
#  StructureSimilarityDialog
#  "structuresimilaritydialog.cpp;similaritygraphwidget.cpp"
#  structuresimilaritydialog.ui
#)
#target_link_libraries(StructureSimilarity
#  vtkGUISupportQt vtkRenderingQt vtkViewsContext2D vtkViewsInfovis
#)
