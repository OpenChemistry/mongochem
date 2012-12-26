mongochem_plugin(ParallelCoordinates
  "Parallel Coordinates"
  AbstractVtkChartWidget
  parallelcoordinatesdialog.h
  ParallelCoordinatesDialog
  parallelcoordinatesdialog.cpp
  parallelcoordinatesdialog.ui
)
target_link_libraries(ParallelCoordinates
  vtkChartsCore vtkRenderingQt
)
