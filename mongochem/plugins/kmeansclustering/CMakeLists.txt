mongochem_plugin(KMeansClustering
  "K-means clustering"
  AbstractClusteringWidget
  kmeansclusteringdialog.h
  KMeansClusteringDialog
  kmeansclusteringdialog.cpp
  kmeansclusteringdialog.ui
)
target_link_libraries(KMeansClustering
  vtkChartsCore vtkGUISupportQt vtkRenderingQt vtkViewsContext2D vtkViewsInfovis
)
