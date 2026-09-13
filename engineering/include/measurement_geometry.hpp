#pragma once
#include "head_geometry.hpp"

namespace starz::engineering {
// Catalogue interface solids, NOT original supplier CAD or manufacturing
// definitions for purchased rubber, bearings, encoder or coupling internals.
TopoDS_Shape measuringWheelInterface(double radial_thread_d_mm=4.0);
TopoDS_Shape measuringEncoderInterface(double face_to_tip_mm=20.0);
TopoDS_Shape measuringCouplingInterface();
TopoDS_Shape measuringBearingInterface();
TopoDS_Shape measuringEncoderPlate();
std::vector<HeadPart> measuringEncoderMount();
void checkMeasuringInterfaces(const TopoDS_Shape& wheel,const TopoDS_Shape& encoder);
void checkMeasuringEncoderMount(const std::vector<HeadPart>& parts);
}