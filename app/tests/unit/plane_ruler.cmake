# SPDX-License-Identifier: MIT
# Fork: the plane ruler's tick spacing and labels: plain Qt Core. It calls
# view/GridSpacing.cpp, which grid_spacing.cmake adds to the target.
target_sources(test_app PRIVATE tests/plane_ruler.cpp view/PlaneRuler.cpp view/PlaneRuler.hpp)
