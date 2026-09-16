#include "fem.hpp"
#include <Eigen/Dense>
#include <Eigen/Sparse>
#include <Eigen/SparseCholesky>
#include <Eigen/CholmodSupport>
#include <gmsh.h>
#include <algorithm>
#include <cmath>
#include <fstream>
#include <iomanip>
#include <map>
#include <numbers>
#include <stdexcept>
#include <unordered_map>

namespace starz::engineering {
namespace {
struct GmshSession {
    GmshSession() { gmsh::initialize(); gmsh::option::setNumber("General.Terminal",0); }
    ~GmshSession() { gmsh::finalize(); }
};
void check(bool ok,const char* message) { if(!ok) throw std::runtime_error(message); }
TetMesh extractMesh(double h,double minimum=0,const std::function<double(const Eigen::Vector3d&)>& sizing={},
                    RefinedMeshOptimizer optimizer=RefinedMeshOptimizer::gmsh) {
    if(!std::isfinite(h)||h<=0) throw std::invalid_argument("Positive mesh size required");
    check(optimizer==RefinedMeshOptimizer::gmsh||optimizer==RefinedMeshOptimizer::netgen,"Unknown refined mesh optimizer");
    gmsh::model::occ::synchronize();
    gmsh::option::setNumber("General.NumThreads",1);
    // Preserve small drilled holes: a global minimum equal to the bulk mesh
    // size can create overlapping boundary facets on curved hole surfaces.
    gmsh::option::setNumber("Mesh.MeshSizeMin",minimum>0?minimum:std::min(0.5,h/8));
    gmsh::option::setNumber("Mesh.MeshSizeMax",h);
    gmsh::option::setNumber("Mesh.MeshSizeFromCurvature",sizing?0:24);
    gmsh::option::setNumber("Mesh.MinimumCirclePoints",24);
    // Curved hole edges must not force sub-mm elements across a metre-long plate.
    gmsh::option::setNumber("Mesh.MeshSizeFromPoints",0);
    gmsh::option::setNumber("Mesh.MeshSizeExtendFromBoundary",0);
    gmsh::option::setNumber("Mesh.ElementOrder",1);
    gmsh::option::setNumber("Mesh.Optimize",1);
    if(sizing) {
        gmsh::option::setNumber("Mesh.Algorithm",5);
        gmsh::model::mesh::setSizeCallback([&](int,int,double x,double y,double z,double lc) {
            const double size=sizing({x,y,z});
            check(std::isfinite(size)&&size>0,"Invalid local mesh size");
            return std::min(lc,size);
        });
    }
    gmsh::model::mesh::generate(3);
    // Optimize the complete volume topology. No cells are filtered and no
    // high-order nodes are generated or silently fed to the linear solver.
    if(optimizer==RefinedMeshOptimizer::netgen)gmsh::model::mesh::optimize("Netgen");
    std::vector<std::size_t> tags; std::vector<double> xyz,param;
    gmsh::model::mesh::getNodes(tags,xyz,param,-1,-1,false,false);
    check(xyz.size()==3*tags.size(),"Malformed mesh coordinates");
    if(sizing)check(tags.size()<=150000,"Over-budget refined FEM mesh");
    TetMesh mesh; std::unordered_map<std::size_t,int> index;
    for(std::size_t i=0;i<tags.size();++i) {
        index.emplace(tags[i],static_cast<int>(i));
        mesh.nodes_mm.emplace_back(xyz[3*i],xyz[3*i+1],xyz[3*i+2]);
    }
    std::vector<std::size_t> elements,nodes;
    gmsh::model::mesh::getElementsByType(4,elements,nodes);
    check(nodes.size()==4*elements.size() && !elements.empty(),"Expected linear tetrahedra");
    for(std::size_t i=0;i<elements.size();++i)
        mesh.tetrahedra.push_back({index.at(nodes[4*i]),index.at(nodes[4*i+1]),index.at(nodes[4*i+2]),index.at(nodes[4*i+3])});
    if(sizing) {
        MeshGeometryAudit audit;gmsh::vectorpair volumes,surfaces;
        gmsh::model::getEntities(volumes,3);gmsh::model::getEntities(surfaces,2);
        for(const auto& [dim,tag]:volumes){double v=0;gmsh::model::occ::getMass(dim,tag,v);audit.cad_volume_mm3+=v;}
        check(std::isfinite(audit.cad_volume_mm3)&&audit.cad_volume_mm3>0,"Invalid CAD mesh volume");
        std::vector<double> quality,cellVolume;
        gmsh::model::mesh::getElementQualities(elements,quality,"minSICN");
        gmsh::model::mesh::getElementQualities(elements,cellVolume,"volume");
        check(quality.size()==elements.size()&&cellVolume.size()==elements.size(),"Missing element quality audit");
        for(std::size_t i=0;i<elements.size();++i) {
            check(std::isfinite(quality[i])&&quality[i]>0&&std::isfinite(cellVolume[i])&&cellVolume[i]>0,"Nonpositive element quality/volume");
            audit.min_sicn=std::min(audit.min_sicn,quality[i]);audit.element_volume_mm3+=cellVolume[i];
        }
        const auto projectionDistance=[](int tag,const std::vector<double>& xyz) {
            check(!xyz.empty()&&xyz.size()%3==0,"Empty/malformed CAD surface sample");
            std::vector<double> projected,parametric;gmsh::model::getClosestPoint(2,tag,xyz,projected,parametric);
            check(projected.size()==xyz.size(),"Missing CAD surface projection");double maximum=0;
            for(std::size_t i=0;i<xyz.size();i+=3) {
                const double distance=std::hypot(xyz[i]-projected[i],xyz[i+1]-projected[i+1],xyz[i+2]-projected[i+2]);
                check(std::isfinite(distance),"Non-finite CAD projection distance");maximum=std::max(maximum,distance);
            }
            return maximum;
        };
        std::vector<std::vector<int>> surfaceMembership(mesh.nodes_mm.size());
        for(const auto& [dim,tag]:surfaces) {
            MeshSurfaceAudit surface;surface.tag=tag;gmsh::model::getType(dim,tag,surface.type);
            std::vector<std::size_t> faceNodes,triangles,triangleNodes;std::vector<double> faceCoordinates,uv;
            gmsh::model::mesh::getNodes(faceNodes,faceCoordinates,uv,2,tag,true,false);surface.nodes=faceNodes.size();
            for(auto node:faceNodes)surfaceMembership[static_cast<std::size_t>(index.at(node))].push_back(tag);
            surface.max_node_distance_mm=projectionDistance(tag,faceCoordinates);
            check(surface.max_node_distance_mm<1e-6,"Refined boundary nodes left CAD surface");
            gmsh::model::mesh::getElementsByType(2,triangles,triangleNodes,tag);surface.triangles=triangles.size();
            check(triangleNodes.size()==3*triangles.size(),"Malformed boundary triangulation");faceCoordinates.clear();
            for(std::size_t i=0;i<triangles.size();++i) {
                Eigen::Vector3d centre=Eigen::Vector3d::Zero();
                for(std::size_t j=0;j<3;++j)centre+=mesh.nodes_mm[static_cast<std::size_t>(index.at(triangleNodes[3*i+j]))]/3;
                for(int j=0;j<3;++j)faceCoordinates.push_back(centre[j]);
            }
            surface.max_facet_centroid_distance_mm=projectionDistance(tag,faceCoordinates);audit.surfaces.push_back(std::move(surface));
        }
        for(std::size_t e=0;e<mesh.tetrahedra.size();++e) {
            const auto& ids=mesh.tetrahedra[e];
            for(int surface:surfaceMembership[static_cast<std::size_t>(ids[0])]) {
                bool common=true;
                for(std::size_t j=1;j<4;++j) {
                    const auto& memberships=surfaceMembership[static_cast<std::size_t>(ids[j])];
                    common=common&&std::find(memberships.begin(),memberships.end(),surface)!=memberships.end();
                }
                if(common){audit.same_surface_tetrahedra.push_back({e,surface});break;}
            }
        }
        mesh.geometry_audit=std::move(audit);
    }
    return mesh;
}
struct Element {
    Eigen::Matrix<double,6,12> b;
    double volume;
};
Element element(const TetMesh& mesh,const std::array<int,4>& ids) {
    Eigen::Matrix4d a;
    for(int i=0;i<4;++i) {
        a(i,0)=1;
        a.block<1,3>(i,1)=mesh.nodes_mm.at(static_cast<std::size_t>(ids[static_cast<std::size_t>(i)])).transpose()*1e-3;
    }
    const double v=std::abs(a.determinant())/6;
    check(std::isfinite(v)&&v>1e-20,"Degenerate tetrahedron");
    const Eigen::Matrix4d inverse=a.inverse();
    Eigen::Matrix<double,6,12> b=Eigen::Matrix<double,6,12>::Zero();
    for(int i=0;i<4;++i) {
        const double x=inverse(1,i),y=inverse(2,i),z=inverse(3,i);
        b(0,3*i)=x; b(1,3*i+1)=y; b(2,3*i+2)=z;
        b(3,3*i)=y; b(3,3*i+1)=x;
        b(4,3*i+1)=z; b(4,3*i+2)=y;
        b(5,3*i)=z; b(5,3*i+2)=x;
    }
    return {b,v};
}
std::vector<std::array<int,3>> boundaryFaces(const TetMesh& mesh) {
    std::map<std::array<int,3>,int> count;
    for(const auto& t:mesh.tetrahedra) for(int omit=0;omit<4;++omit) {
        std::array<int,3> f{}; int j=0;
        for(int i=0;i<4;++i) if(i!=omit) f[static_cast<std::size_t>(j++)]=t[static_cast<std::size_t>(i)];
        std::sort(f.begin(),f.end()); ++count[f];
    }
    std::vector<std::array<int,3>> out;
    for(const auto& [f,n]:count) { check(n==1||n==2,"Non-manifold mesh"); if(n==1) out.push_back(f); }
    return out;
}
Eigen::VectorXd patchVector(const TetMesh& mesh,const std::vector<std::array<int,3>>& faces,const LoadPatch& load) {
    check(static_cast<bool>(load.select)&&load.force_n.allFinite()&&load.couple_nm.allFinite()&&
          (!load.resultant_point_mm||load.resultant_point_mm->allFinite()),"Invalid distributed patch");
    const int n=static_cast<int>(mesh.nodes_mm.size());
    Eigen::VectorXd weights=Eigen::VectorXd::Zero(n),force=Eigen::VectorXd::Zero(3*n);
    for(const auto& f:faces) {
        const auto& a=mesh.nodes_mm[static_cast<std::size_t>(f[0])];
        const auto& b=mesh.nodes_mm[static_cast<std::size_t>(f[1])];
        const auto& c=mesh.nodes_mm[static_cast<std::size_t>(f[2])];
        if(load.select((a+b+c)/3)) {
            const double area=(b-a).cross(c-a).norm()/2;
            for(int id:f)weights[id]+=area/3;
        }
    }
    check(weights.sum()>1e-12,"Distributed patch selected no boundary area");weights/=weights.sum();
    Eigen::Vector3d centre=Eigen::Vector3d::Zero();
    for(int i=0;i<n;++i)centre+=weights[i]*mesh.nodes_mm[static_cast<std::size_t>(i)]*1e-3;
    Eigen::Matrix3d polar=Eigen::Matrix3d::Zero();
    for(int i=0;i<n;++i)if(weights[i]>0) {
        const Eigen::Vector3d r=mesh.nodes_mm[static_cast<std::size_t>(i)]*1e-3-centre;
        polar+=weights[i]*(r.squaredNorm()*Eigen::Matrix3d::Identity()-r*r.transpose());
    }
    check(polar.determinant()>1e-25,"Degenerate distributed moment patch");
    Eigen::Vector3d moment=load.couple_nm;
    if(load.resultant_point_mm)moment+=(*load.resultant_point_mm*1e-3-centre).cross(load.force_n);
    const Eigen::Vector3d spin=polar.ldlt().solve(moment);
    for(int i=0;i<n;++i) {
        const Eigen::Vector3d r=mesh.nodes_mm[static_cast<std::size_t>(i)]*1e-3-centre;
        force.segment<3>(3*i)=weights[i]*(load.force_n+spin.cross(r));
    }
    return force;
}
}
TetMesh meshStep(const std::filesystem::path& file,double h) {
    if(!std::filesystem::is_regular_file(file)) throw std::invalid_argument("STEP mesh input does not exist");
    GmshSession session; gmsh::model::add("selected_part"); gmsh::vectorpair shapes;
    gmsh::model::occ::importShapes(file.string(),shapes,true);
    return extractMesh(h);
}
TetMesh meshBox(double x,double y,double z,double h) {
    if(x<=0||y<=0||z<=0) throw std::invalid_argument("Positive box dimensions required");
    GmshSession session; gmsh::model::add("verification_box"); gmsh::model::occ::addBox(0,0,0,x,y,z);
    return extractMesh(h);
}
TetMesh meshCylinder(double radius,double length,double h) {
    check(std::isfinite(radius)&&radius>0&&std::isfinite(length)&&length>0,"Invalid cylinder dimensions");
    GmshSession session;gmsh::model::add("verification_cylinder");
    gmsh::model::occ::addCylinder(0,0,0,0,length,0,radius);
    return extractMesh(h);
}
TetMesh meshStepRefined(const std::filesystem::path& file,double bulk,double minimum,
                       const std::function<double(const Eigen::Vector3d&)>& sizing,RefinedMeshOptimizer optimizer) {
    check(std::filesystem::is_regular_file(file)&&static_cast<bool>(sizing),"Invalid refined STEP mesh request");
    check(std::isfinite(bulk)&&std::isfinite(minimum)&&minimum>0&&minimum<=bulk,"Invalid refined mesh limits");
    GmshSession session;gmsh::model::add("locally_refined_part");gmsh::vectorpair shapes;
    gmsh::model::occ::importShapes(file.string(),shapes,true);
    return extractMesh(bulk,minimum,sizing,optimizer);
}
std::vector<DistributedElasticResult> solveDistributedElasticCases(
    const TetMesh& mesh,ElasticMaterial mat,const std::vector<DistributedConstraint>& constraints,
    const std::vector<ElasticLoadCase>& cases,double scale) {
    check(std::isfinite(mat.young_pa)&&mat.young_pa>0&&std::isfinite(mat.poisson)&&mat.poisson>-1&&mat.poisson<0.5&&
          std::isfinite(mat.density_kg_m3)&&mat.density_kg_m3>0,"Invalid distributed elastic material");
    check(std::isfinite(scale)&&scale>0,"Invalid constraint stabilisation scale");
    check(!mesh.nodes_mm.empty()&&!mesh.tetrahedra.empty()&&mesh.nodes_mm.size()<=150000,"Invalid or over-budget distributed FEM mesh");
    check(constraints.size()>=6&&constraints.size()<=12&&!cases.empty(),"Distributed model requires 6–12 constraints and load cases");
    for(const auto& p:mesh.nodes_mm)check(p.allFinite(),"Non-finite FEM coordinate");
    const int nodes=static_cast<int>(mesh.nodes_mm.size()),ndof=3*nodes,nc=static_cast<int>(constraints.size());
    const double lambda=mat.young_pa*mat.poisson/((1+mat.poisson)*(1-2*mat.poisson));
    const double mu=mat.young_pa/(2*(1+mat.poisson));
    Eigen::Matrix<double,6,6> D=Eigen::Matrix<double,6,6>::Zero();D.block<3,3>(0,0).setConstant(lambda);
    for(int a=0;a<3;++a){D(a,a)+=2*mu;D(a+3,a+3)=mu;}
    std::vector<Eigen::Triplet<double>> entries;entries.reserve(mesh.tetrahedra.size()*144);
    Eigen::VectorXd mass=Eigen::VectorXd::Zero(nodes);double volume=0;
    for(const auto& t:mesh.tetrahedra) {
        const auto e=element(mesh,t);volume+=e.volume;
        const Eigen::Matrix<double,12,12> local=e.volume*e.b.transpose()*D*e.b;
        for(int i=0;i<4;++i)mass[t[static_cast<std::size_t>(i)]]+=mat.density_kg_m3*e.volume/4;
        for(int i=0;i<12;++i)for(int j=0;j<12;++j)
            entries.emplace_back(3*t[static_cast<std::size_t>(i/3)]+i%3,3*t[static_cast<std::size_t>(j/3)]+j%3,local(i,j));
    }
    Eigen::SparseMatrix<double> K(ndof,ndof);K.setFromTriplets(entries.begin(),entries.end());entries.clear();entries.shrink_to_fit();
    const auto faces=boundaryFaces(mesh);Eigen::MatrixXd C(nc,ndof);Eigen::VectorXd target(nc);
    for(int j=0;j<nc;++j) {
        const auto& constraint=constraints[static_cast<std::size_t>(j)];
        Eigen::VectorXd row=patchVector(mesh,faces,constraint.virtual_load);
        const double norm=row.norm();check(norm>1e-15&&std::isfinite(constraint.value),"Invalid distributed constraint row");
        C.row(j)=row.transpose()/norm;target[j]=constraint.value/norm;
    }
    // Prove that all rigid body modes are controlled before factorisation.
    Eigen::MatrixXd rigid=Eigen::MatrixXd::Zero(ndof,6);
    Eigen::Vector3d centre=Eigen::Vector3d::Zero();for(const auto& p:mesh.nodes_mm)centre+=p/static_cast<double>(nodes);
    for(int i=0;i<nodes;++i) {
        const Eigen::Vector3d r=(mesh.nodes_mm[static_cast<std::size_t>(i)]-centre)*1e-3;
        rigid.block<3,3>(3*i,0)=Eigen::Matrix3d::Identity();
        for(int a=0;a<3;++a)rigid.block<3,1>(3*i,3+a)=Eigen::Vector3d::Unit(a).cross(r);
    }
    for(int a=0;a<6;++a)rigid.col(a).normalize();
    const Eigen::MatrixXd controlled=C*rigid;
    Eigen::ColPivHouseholderQR<Eigen::MatrixXd> rigidRank(controlled);rigidRank.setThreshold(1e-10);
    check(rigidRank.rank()==6,"Distributed constraints leave a rigid mode free");
    const double penalty=scale*K.diagonal().maxCoeff();check(penalty>0&&std::isfinite(penalty),"Invalid augmented stiffness scale");
    // C^T*C would create dense cliques across every support patch. Instead
    // stabilise exactly six independent scalar rigid coordinates. Their
    // residual forces vanish when the rigid equilibrium equations below hold.
    Eigen::ColPivHouseholderQR<Eigen::MatrixXd> pinRank(rigid.transpose());
    pinRank.setThreshold(1e-10);check(pinRank.rank()==6,"Cannot select six rigid gauge coordinates");
    Eigen::SparseMatrix<double> augmented=K;
    for(int j=0;j<6;++j) {
        const auto pin=pinRank.colsPermutation().indices()[j];augmented.coeffRef(pin,pin)+=penalty;
    }
    Eigen::CholmodSupernodalLLT<Eigen::SparseMatrix<double>> solver;
    solver.cholmod().nmethods=1;solver.cholmod().method[0].ordering=CHOLMOD_METIS;
    solver.compute(augmented);
    // Successful LL^T factorization requires a positive pivot at every column;
    // Eigen checks CHOLMOD's completed-column count rather than an LDLT vector.
    check(solver.info()==Eigen::Success&&solver.cholmod().status==CHOLMOD_OK,"Distributed augmented stiffness is unstable");
    Eigen::MatrixXd V;Eigen::FullPivLU<Eigen::MatrixXd> small;
    Eigen::ColPivHouseholderQR<Eigen::MatrixXd> reactionRank;
    if(nc==6) {
        // With exactly six independent constraints, rigid equilibrium fixes
        // every reaction: (C*R)^T*lambda=R^T*f. Solve the balanced elastic
        // load, then use only a rigid motion to meet C*u=g. This is the exact
        // six-constraint specialization, not a relaxed mixed-scale rank test.
        reactionRank.compute(controlled.transpose());reactionRank.setThreshold(1e-10);
        check(reactionRank.rank()==6,"Dependent six-coordinate reactions");
    } else {
        V=solver.solve(C.transpose());check(solver.info()==Eigen::Success&&V.allFinite(),"Constraint influence solve failed");
        const Eigen::MatrixXd schur=C*V;
        Eigen::MatrixXd saddle=Eigen::MatrixXd::Zero(nc+6,nc+6);
        saddle.topLeftCorner(nc,nc)=penalty*(schur+schur.transpose())/2;
        saddle.topRightCorner(nc,6)=-controlled;saddle.bottomLeftCorner(6,nc)=-controlled.transpose();
        small.compute(saddle);small.setThreshold(1e-12);
        check(small.rank()==nc+6,"Dependent distributed constraints");
    }
    std::vector<DistributedElasticResult> results;results.reserve(cases.size());
    for(const auto& load:cases) {
        check(load.body_acceleration_m_s2.allFinite(),"Non-finite case body acceleration");
        Eigen::VectorXd force=Eigen::VectorXd::Zero(ndof);
        for(int i=0;i<nodes;++i)force.segment<3>(3*i)=mass[i]*load.body_acceleration_m_s2;
        for(const auto& patch:load.patches)force+=patchVector(mesh,faces,patch);
        Eigen::VectorXd physicalMultiplier,u;
        if(nc==6) {
            physicalMultiplier=reactionRank.solve(rigid.transpose()*force);
            const Eigen::VectorXd balanced=force-C.transpose()*physicalMultiplier;
            const Eigen::VectorXd elastic=solver.solve(balanced);
            check(solver.info()==Eigen::Success&&elastic.allFinite(),"Balanced distributed elastic solve failed");
            const Eigen::VectorXd motion=rigidRank.solve(target-C*elastic);
            u=elastic+rigid*motion;
        } else {
            const Eigen::VectorXd unconstrained=solver.solve(force);
            check(solver.info()==Eigen::Success&&unconstrained.allFinite(),"Distributed elastic solve failed");
            Eigen::VectorXd rhs(nc+6);rhs.head(nc)=penalty*(C*unconstrained-target);rhs.tail(6)=-rigid.transpose()*force;
            const Eigen::VectorXd smallSolution=small.solve(rhs);
            physicalMultiplier=smallSolution.head(nc);
            u=unconstrained-V*physicalMultiplier+rigid*(smallSolution.tail(6)/penalty);
        }
        const Eigen::VectorXd reaction=-C.transpose()*physicalMultiplier;
        check(u.allFinite()&&reaction.allFinite(),"Non-finite distributed response");
        DistributedElasticResult result;result.volume_m3=volume;result.energy_j=0.5*u.dot(K*u);
        result.equilibrium_relative=(K*u-force-reaction).norm()/std::max(1.,force.norm()+reaction.norm());
        result.constraint_relative=(C*u-target).norm()/std::max({1e-12,u.norm(),target.norm()});
        check(result.equilibrium_relative<1e-7&&result.constraint_relative<1e-7,"Distributed equilibrium/constraint residual failed");
        Eigen::Vector3d totalReaction=Eigen::Vector3d::Zero(),reactionMoment=Eigen::Vector3d::Zero();double fnorm=0,mnorm=0;
        result.support_force_n.assign(constraints.size(),Eigen::Vector3d::Zero());
        result.support_moment_nm.assign(constraints.size(),Eigen::Vector3d::Zero());
        for(int i=0;i<nodes;++i) {
            const Eigen::Vector3d p=mesh.nodes_mm[static_cast<std::size_t>(i)]*1e-3,f=force.segment<3>(3*i),r=reaction.segment<3>(3*i);
            result.total_load_n+=f;result.total_load_moment_nm+=p.cross(f);totalReaction+=r;reactionMoment+=p.cross(r);
            fnorm+=f.norm();mnorm+=p.cross(f).norm();result.displacement_m.emplace_back(u.segment<3>(3*i));
            result.max_displacement_m=std::max(result.max_displacement_m,result.displacement_m.back().norm());
            for(int j=0;j<nc;++j) {
                const Eigen::Vector3d support=-C.block<1,3>(j,3*i).transpose()*physicalMultiplier[j];
                result.support_force_n[static_cast<std::size_t>(j)]+=support;
                result.support_moment_nm[static_cast<std::size_t>(j)]+=p.cross(support);
            }
        }
        result.force_balance_relative=(result.total_load_n+totalReaction).norm()/std::max(1.,fnorm);
        result.moment_balance_relative=(result.total_load_moment_nm+reactionMoment).norm()/std::max(1.,mnorm);
        check(result.force_balance_relative<1e-7&&result.moment_balance_relative<1e-7,"Distributed support resultant balance failed");
        result.stress_pa.reserve(mesh.tetrahedra.size());
        for(const auto& t:mesh.tetrahedra) {
            const auto e=element(mesh,t);Eigen::Matrix<double,12,1> ue;
            for(int j=0;j<4;++j)ue.segment<3>(3*j)=u.segment<3>(3*t[static_cast<std::size_t>(j)]);
            result.stress_pa.emplace_back(D*e.b*ue);
        }
    results.push_back(std::move(result));
    }
    return results;
}
std::vector<ElasticResult> solveElasticCasesImpl(const TetMesh& mesh,ElasticMaterial mat,const Constraints& boundary,
                           const std::vector<FoundationPatch>& foundations,
                           const std::vector<std::vector<LoadPatch>>& loadCases,int modeCount,const Eigen::Vector3d& bodyAcceleration) {
    check(std::isfinite(mat.young_pa)&&mat.young_pa>0&&mat.poisson>-1&&mat.poisson<0.5&&
        std::isfinite(mat.density_kg_m3)&&mat.density_kg_m3>0,"Invalid elastic material");
    check(bodyAcceleration.allFinite(),"Non-finite body acceleration");
    check(!mesh.nodes_mm.empty()&&!mesh.tetrahedra.empty(),"Empty FEM mesh");
    // Bound this in-process direct solver before allocation/fill-in. Large full
    // assemblies need a domain/plate solver, not an unbounded sparse factorisation.
    check(mesh.nodes_mm.size()<=150000,"FEM direct-solver node budget exceeded; use a coarser bulk mesh or domain decomposition");
    check(modeCount>=0 && modeCount<=12,"Invalid requested mode count");
    check(!loadCases.empty(),"At least one load case is required");
    const int count=static_cast<int>(mesh.nodes_mm.size()), ndof=3*count;
    const double lambda=mat.young_pa*mat.poisson/((1+mat.poisson)*(1-2*mat.poisson));
    const double mu=mat.young_pa/(2*(1+mat.poisson));
    Eigen::Matrix<double,6,6> constitutive=Eigen::Matrix<double,6,6>::Zero();
    constitutive.block<3,3>(0,0).setConstant(lambda);
    for(int i=0;i<3;++i) { constitutive(i,i)+=2*mu; constitutive(i+3,i+3)=mu; }
    std::vector<Eigen::Triplet<double>> entries; entries.reserve(mesh.tetrahedra.size()*144);
    Eigen::VectorXd mass=Eigen::VectorXd::Zero(ndof),bodyForce=Eigen::VectorXd::Zero(ndof),prescribed=Eigen::VectorXd::Zero(ndof);
    double totalVolume=0;
    for(const auto& t:mesh.tetrahedra) {
        const auto e=element(mesh,t); totalVolume+=e.volume;
        const Eigen::Matrix<double,12,12> local=e.volume*e.b.transpose()*constitutive*e.b;
        for(int i=0;i<12;++i) {
            const int row=3*t[static_cast<std::size_t>(i/3)]+i%3;
            mass[row]+=mat.density_kg_m3*e.volume/4;
            bodyForce[row]+=mat.density_kg_m3*e.volume/4*bodyAcceleration[i%3];
            for(int j=0;j<12;++j)
                entries.emplace_back(row,3*t[static_cast<std::size_t>(j/3)]+j%3,local(i,j));
        }
    }
    Eigen::SparseMatrix<double> k(ndof,ndof); k.setFromTriplets(entries.begin(),entries.end());
    entries.clear(); entries.shrink_to_fit();
    // Elastic-foundation springs: per-unit-area stiffness distributed over the
    // selected boundary faces as nodal diagonal springs (area/3 tributary, the
    // same face loop convention as the load patches). Faces are tested by
    // centroid; the spring DOFs remain free and enter the reduced system.
    const auto faces=boundaryFaces(mesh);
    Eigen::VectorXd springStiffness=Eigen::VectorXd::Zero(ndof);
    if(!foundations.empty()) {
        for(const auto& f:faces) {
            const auto& a=mesh.nodes_mm[static_cast<std::size_t>(f[0])];
            const auto& b=mesh.nodes_mm[static_cast<std::size_t>(f[1])];
            const auto& c=mesh.nodes_mm[static_cast<std::size_t>(f[2])];
            for(const auto& foundation:foundations) if(foundation.select((a+b+c)/3)) {
                check(foundation.stiffness_pa_per_m.allFinite()&&(foundation.stiffness_pa_per_m.array()>=0).all()&&
                      foundation.stiffness_pa_per_m.norm()>0,"Invalid foundation spring stiffness");
                const double areaM2=(b-a).cross(c-a).norm()/2*1e-6;
                for(int id:f) for(int d=0;d<3;++d) {
                    const double kv=areaM2/3*foundation.stiffness_pa_per_m[static_cast<std::size_t>(d)];
                    if(kv<=0) continue;
                    springStiffness[3*id+d]+=kv; k.coeffRef(3*id+d,3*id+d)+=kv;
                }
                break;
            }
        }
        check(springStiffness.norm()>0,"Foundation patches selected no boundary area");
    }
    std::vector<int> freeIndex(static_cast<std::size_t>(ndof),-1), global;
    for(int i=0;i<count;++i) {
        const auto fixed=boundary?boundary(mesh.nodes_mm[static_cast<std::size_t>(i)]):Constraint{};
        for(int a=0;a<3;++a) {
            const int g=3*i+a;
            if(fixed[static_cast<std::size_t>(a)]) {
                const double value=*fixed[static_cast<std::size_t>(a)];
                check(std::isfinite(value),"Non-finite prescribed displacement"); prescribed[g]=value;
            } else { freeIndex[static_cast<std::size_t>(g)]=static_cast<int>(global.size()); global.push_back(g); }
        }
    }
    const int nf=static_cast<int>(global.size());
    check(nf>0 && (nf<ndof||!foundations.empty()),"Model requires prescribed or elastic-foundation supports");
    Eigen::VectorXd mf(nf);
    for(int i=0;i<nf;++i) mf[i]=mass[global[static_cast<std::size_t>(i)]];
    std::vector<Eigen::Triplet<double>> reduced;
    for(int col=0;col<k.outerSize();++col) for(Eigen::SparseMatrix<double>::InnerIterator it(k,col);it;++it) {
        const int rowFree=freeIndex[static_cast<std::size_t>(it.row())],colFree=freeIndex[static_cast<std::size_t>(it.col())];
        if(rowFree>=0 && colFree>=0) reduced.emplace_back(rowFree,colFree,it.value());
    }
    Eigen::SparseMatrix<double> kf(nf,nf); kf.setFromTriplets(reduced.begin(),reduced.end()); reduced.clear(); reduced.shrink_to_fit();
    Eigen::SimplicialLDLT<Eigen::SparseMatrix<double>> solverLDLT;
    Eigen::CholmodSupernodalLLT<Eigen::SparseMatrix<double>> solverLLT;
    const bool supernodal=!foundations.empty();
    if(supernodal) {
        // The elastic-foundation path keeps every DOF free, so the reduced
        // system is the full nodal system. It uses the supernodal CHOLMOD
        // direct solver already validated on the distributed path; both
        // backends are direct solvers and the 150000-node cap, the
        // equilibrium checks and the result schema are unchanged.
        solverLLT.cholmod().nmethods=1;solverLLT.cholmod().method[0].ordering=CHOLMOD_METIS;
        solverLLT.compute(kf);
        check(solverLLT.info()==Eigen::Success&&solverLLT.cholmod().status==CHOLMOD_OK,"Unstable/unconstrained foundation stiffness matrix");
    } else {
        solverLDLT.compute(kf);
        check(solverLDLT.info()==Eigen::Success && solverLDLT.vectorD().minCoeff()>0,"Unstable/unconstrained or indefinite stiffness matrix");
    }
    const auto solveSystem=[&](const auto& b,auto& x) {
        if(supernodal) {
            x=solverLLT.solve(b);
            check(solverLLT.info()==Eigen::Success&&x.allFinite(),"Elastic solve failed");
        } else {
            x=solverLDLT.solve(b);
            check(solverLDLT.info()==Eigen::Success&&x.allFinite(),"Elastic solve failed");
        }
    };
    std::vector<ElasticResult> results;
    for(const auto& loads:loadCases) {
    ElasticResult result; result.volume_m3=totalVolume;
    Eigen::VectorXd force=bodyForce,u=prescribed;
    Eigen::VectorXd aggregateWeights=Eigen::VectorXd::Zero(count);
    for(const auto& load:loads) {
        check(load.force_n.allFinite()&&load.couple_nm.allFinite()&&
              (!load.resultant_point_mm||load.resultant_point_mm->allFinite()),"Non-finite patch load");
        Eigen::VectorXd weights=Eigen::VectorXd::Zero(count);
        for(const auto& f:faces) {
            const auto& a=mesh.nodes_mm[static_cast<std::size_t>(f[0])];
            const auto& b=mesh.nodes_mm[static_cast<std::size_t>(f[1])];
            const auto& c=mesh.nodes_mm[static_cast<std::size_t>(f[2])];
            if(load.select((a+b+c)/3)) {
                const double area=(b-a).cross(c-a).norm()/2;
                for(int id:f) weights[id]+=area/3;
            }
        }
        check(weights.sum()>1e-12,"Load patch selected no boundary area");
        weights/=weights.sum(); aggregateWeights+=weights;
        Eigen::Vector3d centroid=Eigen::Vector3d::Zero();
        for(int i=0;i<count;++i) centroid+=weights[i]*mesh.nodes_mm[static_cast<std::size_t>(i)]*1e-3;
        Eigen::Vector3d spin=Eigen::Vector3d::Zero();
        if(load.resultant_point_mm || load.couple_nm.squaredNorm()>0) {
            Eigen::Matrix3d polar=Eigen::Matrix3d::Zero();
            for(int i=0;i<count;++i) if(weights[i]>0) {
                const Eigen::Vector3d r=mesh.nodes_mm[static_cast<std::size_t>(i)]*1e-3-centroid;
                polar+=weights[i]*(r.squaredNorm()*Eigen::Matrix3d::Identity()-r*r.transpose());
            }
            check(std::abs(polar.determinant())>1e-25,"Degenerate load moment patch");
            Eigen::Vector3d moment=load.couple_nm;
            if(load.resultant_point_mm) moment+=(*load.resultant_point_mm*1e-3-centroid).cross(load.force_n);
            spin=polar.ldlt().solve(moment);
        }
        for(int i=0;i<count;++i) {
            const Eigen::Vector3d r=mesh.nodes_mm[static_cast<std::size_t>(i)]*1e-3-centroid;
            force.segment<3>(3*i)+=weights[i]*(load.force_n+spin.cross(r));
        }
    }
    Eigen::VectorXd rhs(nf);
    const Eigen::VectorXd lifted=force-k*u;
    for(int i=0;i<nf;++i) rhs[i]=lifted[global[static_cast<std::size_t>(i)]];
    Eigen::VectorXd uf; solveSystem(rhs,uf);
    result.free_residual_relative=(kf*uf-rhs).norm()/std::max(1.,rhs.norm());
    check(result.free_residual_relative<1e-7,"Equilibrium residual exceeds tolerance");
    for(int i=0;i<nf;++i) u[global[static_cast<std::size_t>(i)]]=uf[i];
    const Eigen::VectorXd internal=k*u,reaction=internal-force;
    Eigen::Vector3d totalMoment=Eigen::Vector3d::Zero(); double forceNorm=0,momentNorm=0;
    for(int i=0;i<count;++i) {
        const Eigen::Vector3d v=u.segment<3>(3*i); result.displacement_m.push_back(v);
        result.max_displacement_m=std::max(result.max_displacement_m,v.norm());
        const Eigen::Vector3d f=force.segment<3>(3*i),x=mesh.nodes_mm[static_cast<std::size_t>(i)]*1e-3;
        Eigen::Vector3d r=reaction.segment<3>(3*i);
        // Reactions belong only to prescribed DOFs and to elastic-foundation
        // springs (-k*u on the spring DOFs). Including the remaining free-DOF
        // residual here would make the global balance largely tautological.
        for(int axis=0;axis<3;++axis) {
            const int g=3*i+axis;
            if(freeIndex[static_cast<std::size_t>(g)]>=0)
                r[axis]=springStiffness[static_cast<std::size_t>(g)]>0?-springStiffness[static_cast<std::size_t>(g)]*u[static_cast<std::size_t>(g)]:0.;
        }
        result.total_load_n+=f; result.total_reaction_n+=r;
        result.total_load_moment_nm+=x.cross(f); result.total_reaction_moment_nm+=x.cross(r);
        totalMoment+=x.cross(f+r); forceNorm+=f.norm(); momentNorm+=x.cross(f).norm();
        result.load_average_displacement_m+=aggregateWeights[i]*v.norm();
    }
    if(aggregateWeights.sum()>0) result.load_average_displacement_m/=aggregateWeights.sum();
    result.force_balance_relative=(result.total_load_n+result.total_reaction_n).norm()/std::max(1.,forceNorm);
    result.moment_balance_relative=totalMoment.norm()/std::max(1.,momentNorm);
    check(result.force_balance_relative<1e-7,"Global support force balance failed");
    check(result.moment_balance_relative<1e-7,"Global support moment balance failed");
    result.energy_j=0.5*u.dot(internal);
    std::vector<std::pair<double,double>> distribution;
    for(const auto& t:mesh.tetrahedra) {
        const auto e=element(mesh,t); Eigen::Matrix<double,12,1> ue;
        for(int j=0;j<4;++j) ue.segment<3>(3*j)=u.segment<3>(3*t[static_cast<std::size_t>(j)]);
        const Eigen::Matrix<double,6,1> s=constitutive*e.b*ue;
        const double vm=std::sqrt(0.5*(std::pow(s[0]-s[1],2)+std::pow(s[1]-s[2],2)+std::pow(s[2]-s[0],2))+3*(s[3]*s[3]+s[4]*s[4]+s[5]*s[5]));
        result.von_mises_pa.push_back(vm); result.max_stress_pa=std::max(result.max_stress_pa,vm);
        distribution.emplace_back(vm,e.volume);
    }
    std::sort(distribution.begin(),distribution.end()); double accumulated=0;
    for(const auto& [stress,v]:distribution) { accumulated+=v; if(accumulated>=0.99*result.volume_m3) {result.p99_stress_pa=stress;break;} }
    if(modeCount>0 && results.empty()) {
        const int q=std::min(nf,std::max(modeCount+3,8));
        Eigen::MatrixXd basis(nf,q);
        for(int i=0;i<nf;++i) for(int j=0;j<q;++j) basis(i,j)=std::sin((i+1.)*(j+1.)*0.731)+std::cos((i+2.)*(j+2.)*0.137);
        const auto orthogonalise=[&](Eigen::MatrixXd& v) {
            for(int col=0;col<v.cols();++col) {
                for(int pass=0;pass<2;++pass) for(int j=0;j<col;++j)
                    v.col(col)-=v.col(j)*(v.col(j).array()*mf.array()*v.col(col).array()).sum();
                const double norm=std::sqrt((v.col(col).array().square()*mf.array()).sum());
                check(norm>1e-30,"Modal subspace rank failure"); v.col(col)/=norm;
            }
        };
        orthogonalise(basis);
        for(int iter=0;iter<50;++iter) {
            Eigen::MatrixXd next; solveSystem((basis.array().colwise()*mf.array()).matrix(),next);
            basis=next;
            orthogonalise(basis);
        }
        const Eigen::MatrixXd small=basis.transpose()*kf*basis;
        Eigen::SelfAdjointEigenSolver<Eigen::MatrixXd> eig(small);
        check(eig.info()==Eigen::Success,"Modal Ritz solve failed");
        const Eigen::MatrixXd vectors=basis*eig.eigenvectors();
        for(int j=0;j<modeCount;++j) {
            const double eigenvalue=eig.eigenvalues()[j]; check(eigenvalue>0,"Nonpositive natural frequency");
            const Eigen::VectorXd a=kf*vectors.col(j),b=eigenvalue*(mf.array()*vectors.col(j).array()).matrix();
            result.modal_hz.push_back(std::sqrt(eigenvalue)/(2*std::numbers::pi));
            result.modal_residual.push_back((a-b).norm()/std::max(1e-30,a.norm()+b.norm()));
            check(result.modal_residual.back()<1e-6,"Modal subspace has not converged");
        }
    }
    results.push_back(std::move(result));
    }
    return results;
}
std::vector<ElasticResult> solveElasticCases(const TetMesh& mesh,ElasticMaterial mat,const Constraints& boundary,
                           const std::vector<std::vector<LoadPatch>>& loadCases,int modeCount,const Eigen::Vector3d& bodyAcceleration) {
    return solveElasticCasesImpl(mesh,mat,boundary,{},loadCases,modeCount,bodyAcceleration);
}
std::vector<ElasticResult> solveElasticFoundationCases(const TetMesh& mesh,ElasticMaterial mat,
                           const std::vector<FoundationPatch>& foundations,
                           const std::vector<std::vector<LoadPatch>>& loadCases,int modeCount,const Eigen::Vector3d& bodyAcceleration) {
    return solveElasticCasesImpl(mesh,mat,{},foundations,loadCases,modeCount,bodyAcceleration);
}
ElasticResult solveElastic(const TetMesh& mesh,ElasticMaterial mat,const Constraints& boundary,
                           const std::vector<LoadPatch>& loads,int modeCount,const Eigen::Vector3d& bodyAcceleration) {
    auto results=solveElasticCases(mesh,mat,boundary,{loads},modeCount,bodyAcceleration);
    return std::move(results.front());
}
void writeVtu(const std::filesystem::path& path,const TetMesh& mesh,const ElasticResult& result) {
    std::ofstream f(path); f<<std::setprecision(12);
    f<<"<?xml version=\"1.0\"?>\n<VTKFile type=\"UnstructuredGrid\" version=\"0.1\" byte_order=\"LittleEndian\"><UnstructuredGrid>\n"
     <<"<Piece NumberOfPoints=\""<<mesh.nodes_mm.size()<<"\" NumberOfCells=\""<<mesh.tetrahedra.size()<<"\">\n"
     <<"<Points><DataArray type=\"Float64\" NumberOfComponents=\"3\" format=\"ascii\">\n";
    for(const auto& p:mesh.nodes_mm) f<<p.transpose()<<'\n';
    f<<"</DataArray></Points><Cells><DataArray type=\"Int32\" Name=\"connectivity\" format=\"ascii\">\n";
    for(const auto& t:mesh.tetrahedra) f<<t[0]<<' '<<t[1]<<' '<<t[2]<<' '<<t[3]<<'\n';
    f<<"</DataArray><DataArray type=\"Int32\" Name=\"offsets\" format=\"ascii\">\n";
    for(std::size_t i=1;i<=mesh.tetrahedra.size();++i) f<<4*i<<' ';
    f<<"</DataArray><DataArray type=\"UInt8\" Name=\"types\" format=\"ascii\">\n";
    for(std::size_t i=0;i<mesh.tetrahedra.size();++i) f<<"10 ";
    f<<"</DataArray></Cells><PointData><DataArray type=\"Float64\" Name=\"displacement_mm\" NumberOfComponents=\"3\" format=\"ascii\">\n";
    for(const auto& u:result.displacement_m) f<<(u*1000).transpose()<<'\n';
    f<<"</DataArray></PointData><CellData><DataArray type=\"Float64\" Name=\"von_mises_MPa\" format=\"ascii\">\n";
    for(double s:result.von_mises_pa) f<<s*1e-6<<'\n';
    f<<"</DataArray></CellData></Piece></UnstructuredGrid></VTKFile>\n";
    f.close();
    if(!f) throw std::runtime_error("Failed to write FEM field output");
}
}