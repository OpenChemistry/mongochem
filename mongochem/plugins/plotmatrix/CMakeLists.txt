mongochem_plugin(ScatterPlotMatrix
  "Scatter Plot Matrix"
  AbstractVtkChartWidget
  plotmatrixdialog.h
  PlotMatrixDialog
  plotmatrixdialog.cpp
  plotmatrixdialog.ui
)
target_link_libraries(ScatterPlotMatrix
  vtkChartsCore vtkRenderingQt
)
