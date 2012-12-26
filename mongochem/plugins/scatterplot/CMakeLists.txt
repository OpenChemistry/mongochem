mongochem_plugin(ScatterPlot
  "Scatter Plot"
  AbstractVtkChartWidget
  scatterplotdialog.h
  ScatterPlotDialog
  scatterplotdialog.cpp
  scatterplotdialog.ui
)
target_link_libraries(ScatterPlot
  vtkChartsCore vtkRenderingQt
)
