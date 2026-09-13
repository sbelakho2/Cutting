#pragma once
#include "fem.hpp"
#include <Eigen/Dense>
#include <cmath>
#include <stdexcept>

namespace starz::engineering {
struct BladeFaceMotion {
    double apex_normal_mm, worst_normal_mm, normal_slope_y, normal_slope_z, residual_rms_mm;
};
inline BladeFaceMotion bladeFaceMotion(const TetMesh& mesh,const ElasticResult& result,
                                      double seat_x_mm,double apex_y_mm,double apex_z_mm) {
    std::vector<Eigen::Vector3d> rows; std::vector<double> observations;
    for(std::size_t i=0;i<mesh.nodes_mm.size();++i) {
        const auto& p=mesh.nodes_mm[i];
        if(std::abs(p.x()-seat_x_mm)<1e-5 && p.y()>apex_y_mm-34 && p.y()<apex_y_mm-14 && p.z()>10 && p.z()<43) {
            rows.emplace_back(1,p.y()-apex_y_mm,p.z()-apex_z_mm);
            observations.push_back(result.displacement_m.at(i).x()*1000);
        }
    }
    if(rows.size()<6) throw std::runtime_error("Insufficient blade-seat nodes for normal-motion fit");
    Eigen::MatrixXd a(static_cast<Eigen::Index>(rows.size()),3); Eigen::VectorXd u(static_cast<Eigen::Index>(rows.size()));
    for(std::size_t i=0;i<rows.size();++i) { a.row(static_cast<Eigen::Index>(i))=rows[i].transpose(); u[static_cast<Eigen::Index>(i)]=observations[i]; }
    const auto qr=a.colPivHouseholderQr();
    if(qr.rank()!=3) throw std::runtime_error("Blade-seat motion fit has insufficient rank");
    const Eigen::Vector3d b=qr.solve(u);
    double worst=std::abs(b[0]);
    for(double y:{-30.,19.}) for(double z:{-19.,19.}) worst=std::max(worst,std::abs(b[0]+b[1]*y+b[2]*z));
    return {b[0],worst,b[1],b[2],(a*b-u).norm()/std::sqrt(static_cast<double>(rows.size()))};
}
}