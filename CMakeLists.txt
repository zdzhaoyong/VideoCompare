cmake_minimum_required(VERSION 2.8)
include(cmake/PICMake.cmake)

pi_add_target(VideoCompare BIN src REQUIRED PIL OpenCV Qt System MODULES OpenGL QGLViewer GLUT GLEW)
pi_report_targets()

