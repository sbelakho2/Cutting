#pragma once
#include "fem.hpp"

namespace starz::engineering {
// Geometry-only reader for one ASCII UnstructuredGrid Piece containing linear
// tetrahedra. It does not certify or import displacement/stress field values.
// DTDs/entities, binary/appended geometry, invalid counts/connectivity and
// non-finite coordinates are rejected. The existing150000-node cap is retained.
TetMesh readLinearVtuGeometry(const std::filesystem::path& path);
}