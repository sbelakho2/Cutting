#pragma once
#include <TopoDS_Shape.hxx>

namespace starz::engineering {
struct ShaftSectionProperties {
    double station_mm{},area_mm2{},centroid_x_mm{},centroid_z_mm{};
    double Ixx_mm4{},Izz_mm4{},Ixz_mm4{},minimum_I_mm4{},maximum_I_mm4{};
    double integration_error_estimate{};
};
// Exact B-rep section, bounded-surface integration; no polygon/STL approximation.
// The returned estimate is OCC's numerical integration estimate, not metrology.
ShaftSectionProperties shaftSection(const TopoDS_Shape& shaft,double station_mm);
}