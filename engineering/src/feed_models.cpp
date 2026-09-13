#include "feed_models.hpp"
#include "shared/feed_drive_spec.hpp"
#include <Eigen/Dense>
#include <algorithm>
#include <cmath>
#include <numbers>
#include <stdexcept>

namespace starz::engineering {
namespace {
constexpr double pi=std::numbers::pi;
void positive(double value) {
    if(!std::isfinite(value)||value<=0)throw std::invalid_argument("Positive finite input required");
}
}
double interpolateTable(std::span<const double> x,std::span<const double> y,double at) {
    if(x.size()<2||x.size()!=y.size()||!std::isfinite(at))throw std::invalid_argument("Invalid table");
    for(std::size_t i=0;i<x.size();++i) {
        if(!std::isfinite(x[i])||!std::isfinite(y[i])||(i&&x[i]<=x[i-1]))throw std::invalid_argument("Invalid table entries");
    }
    if(at<x.front()||at>x.back())throw std::out_of_range("No catalogue table extrapolation");
    for(std::size_t i=1;i<x.size();++i) if(at<=x[i])
        return y[i-1]+(y[i]-y[i-1])*(at-x[i-1])/(x[i]-x[i-1]);
    throw std::logic_error("Unreachable interpolation interval");
}
double equalPulleyCentres(double length,double teeth,double pitch) {
    positive(length);positive(teeth);positive(pitch);
    if(std::floor(teeth)!=teeth)throw std::invalid_argument("Integer tooth count required");
    const double c=(length-teeth*pitch)/2;
    if(c<=teeth*pitch/pi)throw std::invalid_argument("Pulley pitch circles overlap");
    return c;
}
double s5mPowerCapacity(double rpm) {
    namespace c=feed::catalogue;
    return interpolateTable(c::power_rpm,c::power_w,rpm)*c::width_factor*c::engagement_factor;
}
double s5mTorqueCapacity(double rpm) {return s5mPowerCapacity(rpm)/(rpm*2*pi/60);}
BeltTension beltTension(double initial,double centres,double length,double radius,double torque,double y) {
    positive(initial);positive(centres);positive(length);positive(radius);
    if(!std::isfinite(torque)||!std::isfinite(y)||y<0)throw std::invalid_argument("Invalid belt load");
    const double half_difference=std::abs(torque)*1000/(2*radius);
    return {centres,0.016*centres,(initial+centres*y/length)/16,
            initial+half_difference,initial-half_difference,2*initial};
}
KeyStress keyStress(double torque,double d,double b,double h,double length) {
    positive(d);positive(b);positive(h);positive(length);
    if(!std::isfinite(torque))throw std::invalid_argument("Invalid torque");
    const double force=2*std::abs(torque)*1000/d;
    return {force/(b*length),force/(h*length)};
}
BeamResult shaftBeam(std::span<const ShaftSegment> sections,double E,double a,double b,
                     std::span<const BeamLoad> loads,int subdivisions) {
    positive(E);
    if(sections.empty()||subdivisions<1||subdivisions>100||!std::isfinite(a)||!std::isfinite(b)||a>=b)
        throw std::invalid_argument("Invalid beam/supports");
    std::vector<double> stations;
    for(std::size_t i=0;i<sections.size();++i) {
        const auto& s=sections[i];positive(s.diameter_mm);
        if(!std::isfinite(s.from_mm)||!std::isfinite(s.to_mm)||s.to_mm<=s.from_mm||
           !std::isfinite(s.hole_mm)||s.hole_mm<0||s.hole_mm>=s.diameter_mm||
           (i&&std::abs(sections[i-1].to_mm-s.from_mm)>1e-9))throw std::invalid_argument("Invalid shaft section");
        // Each constant-EI interval with nodal point loads has an exact cubic
        // Hermite solution. Keep every real section/load/support boundary, but
        // do not create redundant sub-0.5mm elements inside a short relief.
        // Those add round-off amplification without adding beam physics.
        const int elements=std::min(subdivisions,std::max(1,static_cast<int>(std::floor((s.to_mm-s.from_mm)/0.5))));
        for(int j=0;j<=elements;++j)stations.push_back(s.from_mm+(s.to_mm-s.from_mm)*j/elements);
    }
    const double start=sections.front().from_mm,end=sections.back().to_mm;
    if(a<start||b>end)throw std::invalid_argument("Support outside shaft");
    stations.push_back(a);stations.push_back(b);
    for(const auto& l:loads) {
        if(!std::isfinite(l.station_mm)||!std::isfinite(l.force_n)||l.station_mm<start||l.station_mm>end)
            throw std::invalid_argument("Load outside shaft");
        stations.push_back(l.station_mm);
    }
    std::sort(stations.begin(),stations.end());
    stations.erase(std::unique(stations.begin(),stations.end(),[](double u,double v){return std::abs(u-v)<1e-8;}),stations.end());
    const auto index=[&](double x) {
        const auto it=std::find_if(stations.begin(),stations.end(),[&](double s){return std::abs(s-x)<1e-8;});
        if(it==stations.end())throw std::logic_error("Missing beam station");
        return static_cast<Eigen::Index>(it-stations.begin());
    };
    const auto sectionAt=[&](double x)->const ShaftSegment& {
        for(const auto& s:sections)if(x>=s.from_mm-1e-9&&x<=s.to_mm+1e-9)return s;
        throw std::logic_error("Missing shaft material");
    };
    const Eigen::Index nd=static_cast<Eigen::Index>(2*stations.size());
    Eigen::MatrixXd K=Eigen::MatrixXd::Zero(nd,nd);Eigen::VectorXd F=Eigen::VectorXd::Zero(nd);
    for(std::size_t i=0;i+1<stations.size();++i) {
        const double L=stations[i+1]-stations[i];const auto& s=sectionAt((stations[i]+stations[i+1])/2);
        const double I=pi*(std::pow(s.diameter_mm,4)-std::pow(s.hole_mm,4))/64;
        Eigen::Matrix4d k;
        k<<12,6*L,-12,6*L,6*L,4*L*L,-6*L,2*L*L,
           -12,-6*L,12,-6*L,6*L,2*L*L,-6*L,4*L*L;
        K.block<4,4>(static_cast<Eigen::Index>(2*i),static_cast<Eigen::Index>(2*i))+=k*(E*I/(L*L*L));
    }
    for(const auto& l:loads)F[2*index(l.station_mm)]+=l.force_n;
    const auto ia=2*index(a),ib=2*index(b);
    std::vector<Eigen::Index> free;
    for(Eigen::Index i=0;i<nd;++i)if(i!=ia&&i!=ib)free.push_back(i);
    const auto nf=static_cast<Eigen::Index>(free.size());Eigen::MatrixXd k(nf,nf);Eigen::VectorXd f(nf);
    for(Eigen::Index i=0;i<nf;++i) {
        f[i]=F[free[static_cast<std::size_t>(i)]];
        for(Eigen::Index j=0;j<nf;++j)k(i,j)=K(free[static_cast<std::size_t>(i)],free[static_cast<std::size_t>(j)]);
    }
    // Equilibrate translation/rotation DOFs before factorisation. Short relief
    // and chamfer intervals otherwise create a severe stiffness-scale spread.
    const Eigen::VectorXd scale=k.diagonal().array().sqrt().inverse();
    const Eigen::MatrixXd scaled=scale.asDiagonal()*k*scale.asDiagonal();
    Eigen::LDLT<Eigen::MatrixXd> solver(scaled);
    if(solver.info()!=Eigen::Success||!solver.isPositive())throw std::runtime_error("Beam factorisation failed");
    Eigen::VectorXd ufree=scale.cwiseProduct(solver.solve(scale.cwiseProduct(f)));
    const double tolerance=1e-6*std::max(1.0,f.norm());
    for(int refinement=0;refinement<6;++refinement) {
        const Eigen::VectorXd residual=f-k*ufree;
        if(residual.norm()<=tolerance)break;
        const Eigen::VectorXd candidate=ufree+scale.cwiseProduct(solver.solve(scale.cwiseProduct(residual)));
        if((f-k*candidate).norm()>=residual.norm())break;
        ufree=candidate;
    }
    if(!ufree.allFinite()||(k*ufree-f).norm()>tolerance)
        throw std::runtime_error("Beam residual failed: "+std::to_string((k*ufree-f).norm())+" > "+std::to_string(tolerance));
    Eigen::VectorXd u=Eigen::VectorXd::Zero(nd);
    for(Eigen::Index i=0;i<nf;++i)u[free[static_cast<std::size_t>(i)]]=ufree[i];
    const Eigen::VectorXd reaction=K*u-F;BeamResult result;
    result.reaction_a_n=reaction[ia];result.reaction_b_n=reaction[ib];result.stations_mm=stations;
    double sum=result.reaction_a_n+result.reaction_b_n,moment=result.reaction_a_n*a+result.reaction_b_n*b;
    for(const auto& l:loads){sum+=l.force_n;moment+=l.force_n*l.station_mm;}
    result.force_residual_n=std::abs(sum);result.moment_residual_nmm=std::abs(moment);
    for(std::size_t i=0;i<stations.size();++i) {
        result.displacements_mm.push_back(u[static_cast<Eigen::Index>(2*i)]);
        result.slopes_rad.push_back(u[static_cast<Eigen::Index>(2*i+1)]);
        result.max_displacement_mm=std::max(result.max_displacement_mm,std::abs(result.displacements_mm.back()));
    }
    // Evaluate exact piecewise-linear bending moments and all cubic Hermite
    // displacement extrema, not a set of samples that can miss a between-node peak.
    const auto bending=[&](double x) {
        double m=result.reaction_a_n*std::max(0.0,x-a)+result.reaction_b_n*std::max(0.0,x-b);
        for(const auto& l:loads)m+=l.force_n*std::max(0.0,x-l.station_mm);
        return m;
    };
    for(std::size_t i=0;i+1<stations.size();++i) {
        const double L=stations[i+1]-stations[i];const auto& s=sectionAt((stations[i]+stations[i+1])/2);
        const double I=pi*(std::pow(s.diameter_mm,4)-std::pow(s.hole_mm,4))/64;
        const double max_m=std::max(std::abs(bending(stations[i])),std::abs(bending(stations[i+1])));
        result.max_bending_mpa=std::max(result.max_bending_mpa,max_m*s.diameter_mm/(2*I));
        const double v0=result.displacements_mm[i],v1=result.displacements_mm[i+1];
        const double t0=result.slopes_rad[i],t1=result.slopes_rad[i+1];
        const double c1=L*t0,c2=-3*v0-2*L*t0+3*v1-L*t1,c3=2*v0+L*t0-2*v1+L*t1;
        const auto extremum=[&](double q) {
            if(q<=0||q>=1||!std::isfinite(q))return;
            const double v=v0+q*(c1+q*(c2+q*c3));
            result.max_displacement_mm=std::max(result.max_displacement_mm,std::abs(v));
        };
        if(std::abs(c3)<1e-18) {
            if(std::abs(c2)>1e-18)extremum(-c1/(2*c2));
        }else {
            const double discriminant=4*c2*c2-12*c3*c1;
            if(discriminant>=0) {
                const double root=std::sqrt(discriminant);
                extremum((-2*c2+root)/(6*c3));extremum((-2*c2-root)/(6*c3));
            }
        }
    }
    return result;
}
std::vector<ShaftSegment> selectedFeedShaftSections() {
    // Reduced-diameter engineering screens for slots/grooves, not a resolved
    // keyway-root stress calculation. Notch sensitivity is separate in the report.
        return {{0,0.2,11.6,0},{0.2,18.5,12,0},{18.5,19.62,11.43,0},{19.62,31.2,12,0},{31.2,31.5,11.5,0},
            {31.5,33.30,12.3,0},{33.30,61.70,10.95,0},{61.70,64.5,12.3,0},{64.5,73,12.3,0},
            {73,82,15,0},{82,87,19,0},{87,92,12,0},{92,94.45,10,5},
            {94.45,112.55,8.15,5},{112.55,112.8,10,5},{112.8,113,9.6,5.4}};
}
}