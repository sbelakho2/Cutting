#pragma once
#include <TopoDS_Shape.hxx>
#include <array>
#include <filesystem>
#include <string>
#include <vector>

namespace starz::engineering {
struct BoxBounds { double xmin, ymin, zmin, xmax, ymax, zmax; };
struct HeadPart {
    std::string id, description;
    TopoDS_Shape shape;
    bool purchased = false, moving = false;
};
TopoDS_Shape holderShape();
TopoDS_Shape datumShape();
TopoDS_Shape adapterShape();
std::vector<HeadPart> headAssembly(double closure_mm = 0.0);
BoxBounds bounds(const TopoDS_Shape& shape);
double volume(const TopoDS_Shape& shape);
void writeStep(const TopoDS_Shape& shape, const std::filesystem::path& path);
void checkHead(const std::vector<HeadPart>& parts, double closure_mm);
void checkHeadSweep(const std::vector<HeadPart>& open_parts);
}