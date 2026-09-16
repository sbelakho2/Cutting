#include "feed_submodel.hpp"
#include "feed_fem.hpp"
#include "mesh_io.hpp"
#include <BRepAlgoAPI_Common.hxx>
#include <BRepClass3d_SolidClassifier.hxx>
#include <BRepBndLib.hxx>
#include <BRepPrimAPI_MakeBox.hxx>
#include <Bnd_Box.hxx>
#include <TopExp_Explorer.hxx>
#include <TopoDS.hxx>
#include <TopoDS_Compound.hxx>
#include <TopoDS_Solid.hxx>
#include <libxml/parser.h>
#include <libxml/tree.h>
#include <Eigen/CholmodSupport>
#include <Eigen/Dense>
#include <Eigen/Sparse>
#include <algorithm>
#include <array>
#include <charconv>
#include <cmath>
#include <fstream>
#include <iterator>
#include <map>
#include <memory>
#include <numbers>
#include <stdexcept>
#include <string_view>
#include <unordered_set>
#include <vector>

namespace starz::engineering {
namespace {
void require(bool ok,const char* message){if(!ok)throw std::runtime_error(message);}

struct FreeText {void operator()(xmlChar* p) const {xmlFree(p);}};
using Text=std::unique_ptr<xmlChar,FreeText>;
bool named(const xmlNode* node,const char* name){return node&&xmlStrEqual(node->name,BAD_CAST name);}
std::string property(xmlNode* node,const char* name) {
    const Text value(xmlGetProp(node,BAD_CAST name));
    return value?reinterpret_cast<const char*>(value.get()):"";
}
bool whitespace(char c){return c==' '||c=='\t'||c=='\n'||c=='\r';}
std::vector<xmlNode*> children(xmlNode* node) {
    std::vector<xmlNode*> result;
    for(auto* child=node->children;child;child=child->next) {
        require(child->ns==nullptr,"VTU namespaces are unsupported");
        if(child->type==XML_ELEMENT_NODE)result.push_back(child);
        else if(child->type==XML_TEXT_NODE) {
            const Text value(xmlNodeGetContent(child));require(bool(value),"Missing XML text");
            const std::string_view text(reinterpret_cast<const char*>(value.get()));
            require(std::all_of(text.begin(),text.end(),whitespace),"Unexpected VTU text outside an array");
        } else require(child->type==XML_COMMENT_NODE,"Unsupported VTU XML node");
    }
    return result;
}
xmlNode* onlyChild(xmlNode* node,const char* name) {
    const auto list=children(node);require(list.size()==1&&named(list.front(),name),"Expected one VTU child element");return list.front();
}
xmlNode* uniqueChild(xmlNode* node,const char* name) {
    xmlNode* found=nullptr;
    for(auto* child:children(node))if(named(child,name)){require(!found,"Duplicate VTU section");found=child;}
    require(found!=nullptr,"Missing VTU section");return found;
}
xmlNode* namedArray(xmlNode* node,const char* name) {
    xmlNode* found=nullptr;
    for(auto* child:children(node)) {
        require(named(child,"DataArray"),"Non-array VTU data section");
        if(property(child,"Name")==name){require(!found,"Duplicate VTU data array");found=child;}
    }
    require(found!=nullptr,"Missing VTU data array");return found;
}
Text arrayText(xmlNode* array,const char* type,const char* components) {
    require(named(array,"DataArray")&&property(array,"format")=="ascii"&&property(array,"type")==type,"Unsupported VTU field array encoding/type");
    const auto width=property(array,"NumberOfComponents");
    require(width==components||(width.empty()&&std::string_view(components)=="1"),"Wrong VTU field component count");
    for(auto* child=array->children;child;child=child->next)
        require(child->type==XML_TEXT_NODE||child->type==XML_CDATA_SECTION_NODE,"Nested VTU array content is unsupported");
    Text text(xmlNodeGetContent(array));require(bool(text),"Missing VTU array values");return text;
}
template<class Consumer> void parseNumbers(const Text& text,std::size_t expected,Consumer consume) {
    const std::string_view view(reinterpret_cast<const char*>(text.get()));const char* next=view.data();const char* end=next+view.size();
    for(std::size_t i=0;i<expected;++i) {
        while(next!=end&&whitespace(*next))++next;
        double value=0;const auto result=std::from_chars(next,end,value);
        require(result.ec==std::errc{}&&result.ptr!=next&&(result.ptr==end||whitespace(*result.ptr)),"Malformed or missing VTU number");
        require(std::isfinite(value),"Non-finite VTU field value");
        consume(i,value);next=result.ptr;
    }
    while(next!=end&&whitespace(*next))++next;require(next==end,"Excess VTU field values");
}

std::vector<std::array<int,3>> boundaryFaces(const TetMesh& mesh) {
    std::map<std::array<int,3>,int> count;
    for(const auto& t:mesh.tetrahedra)for(int omit=0;omit<4;++omit) {
        std::array<int,3> f{};int j=0;
        for(int i=0;i<4;++i)if(i!=omit)f[static_cast<std::size_t>(j++)]=t[static_cast<std::size_t>(i)];
        std::sort(f.begin(),f.end());++count[f];
    }
    std::vector<std::array<int,3>> out;
    for(const auto& [f,n]:count){require(n==1||n==2,"Non-manifold submodel mesh");if(n==1)out.push_back(f);}
    return out;
}
} // namespace

VtuField readLinearVtuField(const std::filesystem::path& path) {
    VtuField field;field.mesh=readLinearVtuGeometry(path);
    require(std::filesystem::is_regular_file(path),"Missing VTU field input");
    std::ifstream file(path,std::ios::binary);require(bool(file),"VTU field input cannot be opened");
    const std::string input{std::istreambuf_iterator<char>(file),std::istreambuf_iterator<char>()};require(!file.bad(),"VTU field input read failed");
    const auto options=XML_PARSE_NONET|XML_PARSE_HUGE|XML_PARSE_NOERROR|XML_PARSE_NOWARNING;
    const std::unique_ptr<xmlDoc,decltype(&xmlFreeDoc)> document(xmlReadMemory(input.data(),static_cast<int>(input.size()),path.c_str(),"UTF-8",options),xmlFreeDoc);
    require(bool(document),"Invalid VTU field XML");
    auto* root=xmlDocGetRootElement(document.get());require(named(root,"VTKFile"),"Expected VTU root element");
    auto* grid=onlyChild(root,"UnstructuredGrid");auto* piece=onlyChild(grid,"Piece");
    auto* pointData=uniqueChild(piece,"PointData");auto* cellData=uniqueChild(piece,"CellData");
    const std::size_t nodes=field.mesh.nodes_mm.size(),cells=field.mesh.tetrahedra.size();
    const auto displacement=arrayText(namedArray(pointData,"displacement_mm"),"Float64","3");
    field.displacement_mm.resize(nodes);
    parseNumbers(displacement,3*nodes,[&](std::size_t i,double value){field.displacement_mm[i/3][static_cast<int>(i%3)]=value;});
    const auto stress=arrayText(namedArray(cellData,"von_mises_MPa"),"Float64","1");
    field.von_mises_mpa.resize(cells);
    parseNumbers(stress,cells,[&](std::size_t i,double value){field.von_mises_mpa[i]=value;});
    return field;
}

struct DisplacementFieldInterpolator::Impl {
    TetMesh mesh;
    std::vector<std::vector<Eigen::Vector3d>> fields;
    std::vector<double> inverse_det;         // per tet
    std::vector<std::array<double,16>> adjugate; // per tet (row-major 4x4)
    double cell_size=1.0,boundary_tolerance=0.02;
    Eigen::Vector3d lower=Eigen::Vector3d::Zero();
    std::array<int,3> grid={{0,0,0}};
    std::vector<std::vector<int>> buckets;
    mutable DisplacementInterpolationAudit audit;
    mutable std::vector<int> scratch;

    void bucketFor(const Eigen::Vector3d& p,int& i,int& j,int& k) const {
        i=std::clamp(static_cast<int>(std::floor((p.x()-lower.x())/cell_size)),0,grid[0]-1);
        j=std::clamp(static_cast<int>(std::floor((p.y()-lower.y())/cell_size)),0,grid[1]-1);
        k=std::clamp(static_cast<int>(std::floor((p.z()-lower.z())/cell_size)),0,grid[2]-1);
    }
    static double circumdiameter(const TetMesh& m,const std::array<int,4>& t) {
        // Simplified scale proxy: twice the longest edge; only used to size the grid.
        double longest=0;
        for(int i=0;i<4;++i)for(int j=i+1;j<4;++j)
            longest=std::max(longest,(m.nodes_mm[static_cast<std::size_t>(t[static_cast<std::size_t>(i)])]-m.nodes_mm[static_cast<std::size_t>(t[static_cast<std::size_t>(j)])]).norm());
        return longest;
    }
    static void shapeCoefficients(const TetMesh& m,const std::array<int,4>& t,double& det,std::array<double,16>& inverseTranspose) {
        Eigen::Matrix4d a;
        for(int i=0;i<4;++i) {
            a(i,0)=1;
            const auto& p=m.nodes_mm.at(static_cast<std::size_t>(t[static_cast<std::size_t>(i)]));
            a(i,1)=p.x();a(i,2)=p.y();a(i,3)=p.z();
        }
        det=a.determinant();require(std::isfinite(det)&&std::abs(det)>1e-12,"Degenerate global tetrahedron");
        const Eigen::Matrix4d solved=a.inverse();
        // Barycentric weights are lambda = A^-T * (1,x,y,z)^T; store the
        // inverse transpose so that lambda_i = sum_j stored[4*i+j]*b[j].
        for(int i=0;i<4;++i)for(int j=0;j<4;++j)inverseTranspose[static_cast<std::size_t>(4*i+j)]=solved(j,i);
    }
    // Barycentric weights of p in tet index e; returns false when p lies
    // outside (any weight < -eps or sum off).
    bool barycentric(int e,const Eigen::Vector3d& p,std::array<double,4>& weights) const {
        const auto& inverse=adjugate[static_cast<std::size_t>(e)];
        const double b[4]={1,p.x(),p.y(),p.z()};
        for(int i=0;i<4;++i) {
            double w=0;
            for(int j=0;j<4;++j)w+=inverse[static_cast<std::size_t>(4*i+j)]*b[j];
            weights[static_cast<std::size_t>(i)]=w;
        }
        double sum=0;for(double w:weights)sum+=w;
        if(std::abs(sum-1)>1e-7)return false;
        for(double w:weights)if(w<-1e-9)return false;
        return true;
    }
    // Closest point on tetrahedron (faces only; Ericson, Real-Time Collision
    // Detection, 2005). Returns distance and barycentric weights of the
    // closest point; the point lies on the tet hull.
    double closestPoint(int e,const Eigen::Vector3d& p,Eigen::Vector3d& closest,std::array<double,4>& weights) const {
        const auto& t=mesh.tetrahedra[static_cast<std::size_t>(e)];
        Eigen::Vector3d best=Eigen::Vector3d::Zero();double bestDistance=std::numeric_limits<double>::infinity();
        for(int omit=0;omit<4;++omit) {
            const int a=t[static_cast<std::size_t>((omit+1)%4)],b=t[static_cast<std::size_t>((omit+2)%4)],c=t[static_cast<std::size_t>((omit+3)%4)];
            const auto& A=mesh.nodes_mm[static_cast<std::size_t>(a)],B=mesh.nodes_mm[static_cast<std::size_t>(b)],C=mesh.nodes_mm[static_cast<std::size_t>(c)];
            const Eigen::Vector3d ab=B-A,ac=C-A,ap=p-A;
            const double d1=ab.dot(ap),d2=ac.dot(ap);
            if(d1<=0&&d2<=0) { const Eigen::Vector3d q=A;const double d=(p-q).norm(); if(d<bestDistance){bestDistance=d;best=q;} continue; }
            const Eigen::Vector3d bp=p-B;
            const double d3=ab.dot(bp),d4=ac.dot(bp);
            if(d3>=0&&d4<=d3) { const Eigen::Vector3d q=B;const double d=(p-q).norm(); if(d<bestDistance){bestDistance=d;best=q;} continue; }
            const double vc=d1*d4-d3*d2;
            if(vc<=0&&d1>=0&&d3<=0) { const double v=d1/(d1-d3);const Eigen::Vector3d q=A+v*ab;const double d=(p-q).norm(); if(d<bestDistance){bestDistance=d;best=q;} continue; }
            const Eigen::Vector3d cp=p-C;
            const double d5=ab.dot(cp),d6=ac.dot(cp);
            if(d6>=0&&d5<=d6) { const Eigen::Vector3d q=C;const double d=(p-q).norm(); if(d<bestDistance){bestDistance=d;best=q;} continue; }
            const double vb=d5*d2-d1*d6;
            if(vb<=0&&d2>=0&&d6<=0) { const double w=d2/(d2-d6);const Eigen::Vector3d q=A+w*ac;const double d=(p-q).norm(); if(d<bestDistance){bestDistance=d;best=q;} continue; }
            const double va=d3*d6-d5*d4;
            if(va<=0&&(d4-d3)>=0&&(d5-d6)>=0) { const double w=(d4-d3)/((d4-d3)+(d5-d6));const Eigen::Vector3d q=B+w*(C-B);const double d=(p-q).norm(); if(d<bestDistance){bestDistance=d;best=q;} continue; }
            const double denom=1/(va+vb+vc);const double v=vb*denom,w=vc*denom;const Eigen::Vector3d q=A+ab*v+ac*w;
            const double d=(p-q).norm(); if(d<bestDistance){bestDistance=d;best=q;}
        }
        closest=best;
        // Weights of the hull point through the stored inverse; clamp
        // roundoff-scale negatives and renormalise to unit sum.
        const auto& inverse=adjugate[static_cast<std::size_t>(e)];
        const double b[4]={1,best.x(),best.y(),best.z()};
        double sum=0;
        for(int i=0;i<4;++i) {
            double w=0;
            for(int j=0;j<4;++j)w+=inverse[static_cast<std::size_t>(4*i+j)]*b[j];
            weights[static_cast<std::size_t>(i)]=std::max(0.,w);
            sum+=weights[static_cast<std::size_t>(i)];
        }
        require(sum>1e-12,"Hull projection weights degenerate");
        for(double& w:weights)w/=sum;
        return bestDistance;
    }
    std::vector<Eigen::Vector3d> gather(const std::array<double,4>& weights,const std::array<int,4>& t) const {
        std::vector<Eigen::Vector3d> out(fields.size(),Eigen::Vector3d::Zero());
        for(std::size_t f=0;f<fields.size();++f)for(int i=0;i<4;++i)
            out[f]+=weights[static_cast<std::size_t>(i)]*fields[f][static_cast<std::size_t>(t[static_cast<std::size_t>(i)])];
        return out;
    }
};

DisplacementFieldInterpolator::DisplacementFieldInterpolator():impl_(std::make_unique<Impl>()) {}
DisplacementFieldInterpolator::~DisplacementFieldInterpolator()=default;

void DisplacementFieldInterpolator::build(const TetMesh& mesh,const std::vector<std::vector<Eigen::Vector3d>>& basis_fields_mm,
                                          double boundary_tolerance_mm) {
    require(!mesh.nodes_mm.empty()&&!mesh.tetrahedra.empty(),"Empty interpolation mesh");
    require(basis_fields_mm.size()>=1&&basis_fields_mm.size()<=8,"Interpolation field count out of range");
    require(std::isfinite(boundary_tolerance_mm)&&boundary_tolerance_mm>0,"Invalid interpolation boundary tolerance");
    for(const auto& f:basis_fields_mm)require(f.size()==mesh.nodes_mm.size(),"Interpolation field size mismatch");
    impl_->mesh=mesh;impl_->fields=basis_fields_mm;impl_->boundary_tolerance=boundary_tolerance_mm;
    impl_->inverse_det.resize(mesh.tetrahedra.size());impl_->adjugate.resize(mesh.tetrahedra.size());
    double longest=0;
    for(std::size_t e=0;e<mesh.tetrahedra.size();++e) {
        Impl::shapeCoefficients(mesh,mesh.tetrahedra[e],impl_->inverse_det[e],impl_->adjugate[e]);
        longest=std::max(longest,Impl::circumdiameter(mesh,mesh.tetrahedra[e]));
    }
    impl_->cell_size=std::max(0.05,longest/2);
    Eigen::Vector3d upper=mesh.nodes_mm[0];
    for(const auto& p:mesh.nodes_mm){impl_->lower=impl_->lower.cwiseMin(p);upper=upper.cwiseMax(p);}
    impl_->lower-=Eigen::Vector3d::Constant(impl_->cell_size);
    for(int axis=0;axis<3;++axis)impl_->grid[static_cast<std::size_t>(axis)]=std::max(1,
        static_cast<int>(std::ceil((upper[axis]+impl_->cell_size-impl_->lower[axis])/impl_->cell_size))+1);
    impl_->buckets.assign(static_cast<std::size_t>(impl_->grid[0])*static_cast<std::size_t>(impl_->grid[1])*static_cast<std::size_t>(impl_->grid[2]),{});
    for(std::size_t e=0;e<mesh.tetrahedra.size();++e) {
        Eigen::Vector3d lo=mesh.nodes_mm[static_cast<std::size_t>(mesh.tetrahedra[e][0])],hi=lo;
        for(int i=1;i<4;++i){const auto& p=mesh.nodes_mm[static_cast<std::size_t>(mesh.tetrahedra[e][static_cast<std::size_t>(i)])];lo=lo.cwiseMin(p);hi=hi.cwiseMax(p);}
        int i0,j0,k0,i1,j1,k1;impl_->bucketFor(lo,i0,j0,k0);impl_->bucketFor(hi,i1,j1,k1);
        for(int i=i0;i<=i1;++i)for(int j=j0;j<=j1;++j)for(int k=k0;k<=k1;++k)
            impl_->buckets[(static_cast<std::size_t>(k)*static_cast<std::size_t>(impl_->grid[1])+static_cast<std::size_t>(j))*static_cast<std::size_t>(impl_->grid[0])+static_cast<std::size_t>(i)].push_back(static_cast<int>(e));
    }
    impl_->audit=DisplacementInterpolationAudit{};impl_->audit.cell_size_mm=impl_->cell_size;
    impl_->scratch.clear();
}

Eigen::Vector3d DisplacementFieldInterpolator::interpolate(std::size_t field,const Eigen::Vector3d& point_mm) const {
    require(field<impl_->fields.size(),"Interpolation field index out of range");
    require(point_mm.allFinite(),"Non-finite interpolation point");
    impl_->scratch.clear();
    int i,j,k;impl_->bucketFor(point_mm,i,j,k);
    for(int di=-1;di<=1;++di)for(int dj=-1;dj<=1;++dj)for(int dk=-1;dk<=1;++dk) {
        const int ii=i+di,jj=j+dj,kk=k+dk;
        if(ii<0||jj<0||kk<0||ii>=impl_->grid[0]||jj>=impl_->grid[1]||kk>=impl_->grid[2])continue;
        for(int e:impl_->buckets[(static_cast<std::size_t>(kk)*static_cast<std::size_t>(impl_->grid[1])+static_cast<std::size_t>(jj))*static_cast<std::size_t>(impl_->grid[0])+static_cast<std::size_t>(ii)])
            impl_->scratch.push_back(e);
    }
    std::sort(impl_->scratch.begin(),impl_->scratch.end());
    impl_->scratch.erase(std::unique(impl_->scratch.begin(),impl_->scratch.end()),impl_->scratch.end());
    std::array<double,4> weights;
    for(int e:impl_->scratch)if(impl_->barycentric(e,point_mm,weights)) {
        const auto values=impl_->gather(weights,impl_->mesh.tetrahedra[static_cast<std::size_t>(e)]);
        ++impl_->audit.exact_hits;
        return values[field];
    }
    // Outside the flat-facet hull: nearest-hull projection within tolerance.
    double best=std::numeric_limits<double>::infinity();int bestTet=-1;std::array<double,4> bestWeights{};Eigen::Vector3d closest;
    for(int e:impl_->scratch) {
        Eigen::Vector3d q;std::array<double,4> w;
        const double d=impl_->closestPoint(e,point_mm,q,w);
        if(d<best){best=d;bestTet=e;bestWeights=w;closest=q;}
    }
    require(bestTet>=0,"Interpolation point has no candidate element");
    if(best<=impl_->boundary_tolerance) {
        const auto values=impl_->gather(bestWeights,impl_->mesh.tetrahedra[static_cast<std::size_t>(bestTet)]);
        ++impl_->audit.boundary_projections;
        impl_->audit.max_gap_mm=std::max(impl_->audit.max_gap_mm,best);
        return values[field];
    }
    ++impl_->audit.unsupported;
    throw std::runtime_error("Submodel boundary point outside global displacement support: nearest facet gap "+
                             std::to_string(best)+" mm exceeds tolerance "+std::to_string(impl_->boundary_tolerance)+" mm at "+
                             std::to_string(point_mm.x())+","+std::to_string(point_mm.y())+","+std::to_string(point_mm.z()));
}

Eigen::Vector3d DisplacementFieldInterpolator::combine(const std::array<double,8>& weights,const Eigen::Vector3d& point_mm) const {
    impl_->scratch.clear();
    int i,j,k;impl_->bucketFor(point_mm,i,j,k);
    for(int di=-1;di<=1;++di)for(int dj=-1;dj<=1;++dj)for(int dk=-1;dk<=1;++dk) {
        const int ii=i+di,jj=j+dj,kk=k+dk;
        if(ii<0||jj<0||kk<0||ii>=impl_->grid[0]||jj>=impl_->grid[1]||kk>=impl_->grid[2])continue;
        for(int e:impl_->buckets[(static_cast<std::size_t>(kk)*static_cast<std::size_t>(impl_->grid[1])+static_cast<std::size_t>(jj))*static_cast<std::size_t>(impl_->grid[0])+static_cast<std::size_t>(ii)])
            impl_->scratch.push_back(e);
    }
    std::sort(impl_->scratch.begin(),impl_->scratch.end());
    impl_->scratch.erase(std::unique(impl_->scratch.begin(),impl_->scratch.end()),impl_->scratch.end());
    std::array<double,4> bary;
    for(int e:impl_->scratch)if(impl_->barycentric(e,point_mm,bary)) {
        const auto values=impl_->gather(bary,impl_->mesh.tetrahedra[static_cast<std::size_t>(e)]);
        Eigen::Vector3d out=Eigen::Vector3d::Zero();
        for(std::size_t f=0;f<impl_->fields.size();++f)out+=weights[f]*values[f];
        ++impl_->audit.exact_hits;
        return out;
    }
    double best=std::numeric_limits<double>::infinity();int bestTet=-1;std::array<double,4> bestWeights{};
    for(int e:impl_->scratch) {
        Eigen::Vector3d q;std::array<double,4> w;
        const double d=impl_->closestPoint(e,point_mm,q,w);
        if(d<best){best=d;bestTet=e;bestWeights=w;}
    }
    require(bestTet>=0,"Interpolation point has no candidate element");
    if(best<=impl_->boundary_tolerance) {
        const auto values=impl_->gather(bestWeights,impl_->mesh.tetrahedra[static_cast<std::size_t>(bestTet)]);
        Eigen::Vector3d out=Eigen::Vector3d::Zero();
        for(std::size_t f=0;f<impl_->fields.size();++f)out+=weights[f]*values[f];
        ++impl_->audit.boundary_projections;
        impl_->audit.max_gap_mm=std::max(impl_->audit.max_gap_mm,best);
        return out;
    }
    ++impl_->audit.unsupported;
    throw std::runtime_error("Submodel boundary point outside global displacement support: nearest facet gap "+
                             std::to_string(best)+" mm exceeds tolerance "+std::to_string(impl_->boundary_tolerance)+" mm at "+
                             std::to_string(point_mm.x())+","+std::to_string(point_mm.y())+","+std::to_string(point_mm.z()));
}

const DisplacementInterpolationAudit& DisplacementFieldInterpolator::audit() const {return impl_->audit;}

SubmodelSlab cutShaftSlab(const TopoDS_Shape& shaft,double station_mm,double half_mm) {
    require(std::isfinite(station_mm)&&std::isfinite(half_mm)&&half_mm>=2&&half_mm<=10,"Invalid slab window");
    Bnd_Box bounds;BRepBndLib::Add(shaft,bounds);
    double xmin,ymin,zmin,xmax,ymax,zmax;bounds.Get(xmin,ymin,zmin,xmax,ymax,zmax);
    const double pad=2.0;
    require(station_mm-half_mm>ymin+1e-4&&station_mm+half_mm<ymax-1e-4,"Slab window exceeds the shaft axial extent");
    // The box spans exactly the window in S; x/z are padded beyond the shaft
    // surface so the cut faces are the two planar window disks.
    BRepPrimAPI_MakeBox box(gp_Pnt(xmin-pad,station_mm-half_mm,zmin-pad),gp_Pnt(xmax+pad,station_mm+half_mm,zmax+pad));
    require(!box.Shape().IsNull(),"Slab cutting box construction failed");
    BRepAlgoAPI_Common cut(shaft,box.Shape());
    require(cut.IsDone()&&!cut.Shape().IsNull(),"Slab boolean cut failed");
    TopoDS_Shape result=cut.Shape();
    int solids=0;for(TopExp_Explorer explorer(result,TopAbs_SOLID);explorer.More();explorer.Next())++solids;
    require(solids==1,"Slab boolean produced no single solid");
    // The boolean's face trims can inflate the bounding box (observed 0.16-0.19 mm
    // near the washer-slot walls), so the window is verified by point
    // classification: the axis must be solid strictly inside the window and
    // empty strictly outside it.
    BRepClass3d_SolidClassifier classifier(result);
    for(const double probe:std::array<double,4>{station_mm-(half_mm-1),station_mm+(half_mm-1),
                                                station_mm-(half_mm+1),station_mm+(half_mm+1)}) {
        const bool inside=std::abs(probe-station_mm)<half_mm;
        // Probe at r=3.5 mm: inside the solid for every retained window even
        // where the retaining drill (r2.1) removes the axis material.
        classifier.Perform(gp_Pnt(3.5,probe,0),1e-7);
        require((classifier.State()==TopAbs_IN)==inside,"Slab window classification disagrees with the cut planes");
    }
    return {result,station_mm,half_mm};
}

SubmodelCutClassification classifySlabCutNodes(const TetMesh& submesh,const SubmodelSlab& slab,double plane_tolerance_mm) {
    require(std::isfinite(plane_tolerance_mm)&&plane_tolerance_mm>0,"Invalid cut-plane tolerance");
    const double rear=slab.station_mm-slab.half_mm,front=slab.station_mm+slab.half_mm;
    SubmodelCutClassification classification;classification.constrained.assign(submesh.nodes_mm.size(),0);
    for(const auto& face:boundaryFaces(submesh)) {
        bool onRear=true,onFront=true;
        for(int n:face) {
            const double y=submesh.nodes_mm[static_cast<std::size_t>(n)].y();
            onRear=onRear&&std::abs(y-rear)<plane_tolerance_mm;
            onFront=onFront&&std::abs(y-front)<plane_tolerance_mm;
        }
        if(onRear||onFront)for(int n:face)classification.constrained[static_cast<std::size_t>(n)]=1;
    }
    classification.cut_nodes=static_cast<std::size_t>(std::count(classification.constrained.begin(),classification.constrained.end(),1));
    require(classification.cut_nodes>0,"Slab submodel has no cut-face nodes");
    std::unordered_set<int> boundaryNodes;
    for(const auto& face:boundaryFaces(submesh))for(int n:face)boundaryNodes.insert(n);
    classification.boundary_nodes=boundaryNodes.size();
    require(classification.cut_nodes<boundaryNodes.size(),"Every submodel boundary node classified as cut");
    return classification;
}

std::vector<SubmodelSolveResult> solveSubmodelCases(const TetMesh& mesh,ElasticMaterial mat,const std::vector<SubmodelCase>& cases) {
    require(std::isfinite(mat.young_pa)&&mat.young_pa>0&&mat.poisson>-1&&mat.poisson<0.5&&
            std::isfinite(mat.density_kg_m3)&&mat.density_kg_m3>0,"Invalid submodel material");
    require(!mesh.nodes_mm.empty()&&!mesh.tetrahedra.empty(),"Empty submodel mesh");
    require(mesh.nodes_mm.size()<=150000,"Submodel direct-solver node budget exceeded");
    require(cases.size()>=1&&cases.size()<=8,"Submodel case count out of range");
    const int count=static_cast<int>(mesh.nodes_mm.size()),ndof=3*count;
    for(const auto& c:cases)require(c.prescribed_m.size()==mesh.nodes_mm.size(),"Submodel prescribed field size mismatch");
    for(const auto& p:mesh.nodes_mm)require(p.allFinite(),"Non-finite submodel coordinate");
    const double lambda=mat.young_pa*mat.poisson/((1+mat.poisson)*(1-2*mat.poisson));
    const double mu=mat.young_pa/(2*(1+mat.poisson));
    Eigen::Matrix<double,6,6> D=Eigen::Matrix<double,6,6>::Zero();D.block<3,3>(0,0).setConstant(lambda);
    for(int a=0;a<3;++a){D(a,a)+=2*mu;D(a+3,a+3)=mu;}
    std::vector<Eigen::Triplet<double>> entries;entries.reserve(mesh.tetrahedra.size()*144);
    Eigen::VectorXd mass=Eigen::VectorXd::Zero(count);
    double totalVolume=0;
    for(const auto& t:mesh.tetrahedra) {
        Eigen::Matrix4d a;
        for(int i=0;i<4;++i){a(i,0)=1;a.block<1,3>(i,1)=mesh.nodes_mm.at(static_cast<std::size_t>(t[static_cast<std::size_t>(i)])).transpose()*1e-3;}
        const double v=std::abs(a.determinant())/6;
        require(std::isfinite(v)&&v>1e-20,"Degenerate submodel tetrahedron");
        const Eigen::Matrix4d inverse=a.inverse();
        Eigen::Matrix<double,6,12> b=Eigen::Matrix<double,6,12>::Zero();
        for(int i=0;i<4;++i) {
            const double x=inverse(1,i),y=inverse(2,i),z=inverse(3,i);
            b(0,3*i)=x;b(1,3*i+1)=y;b(2,3*i+2)=z;
            b(3,3*i)=y;b(3,3*i+1)=x;
            b(4,3*i+1)=z;b(4,3*i+2)=y;
            b(5,3*i)=z;b(5,3*i+2)=x;
        }
        totalVolume+=v;
        const Eigen::Matrix<double,12,12> local=v*b.transpose()*D*b;
        for(int i=0;i<12;++i) {
            const int row=3*t[static_cast<std::size_t>(i/3)]+i%3;
            mass[row]+=mat.density_kg_m3*v/4;
            for(int j=0;j<12;++j)
                entries.emplace_back(row,3*t[static_cast<std::size_t>(j/3)]+j%3,local(i,j));
        }
    }
    Eigen::SparseMatrix<double> k(ndof,ndof);k.setFromTriplets(entries.begin(),entries.end());
    entries.clear();entries.shrink_to_fit();
    std::vector<int> freeIndex(static_cast<std::size_t>(ndof),-1),global;
    for(const auto& c:cases)for(int i=0;i<count;++i)for(int axis=0;axis<3;++axis)
        require(c.prescribed_m[static_cast<std::size_t>(i)][static_cast<std::size_t>(axis)].has_value()==
                cases.front().prescribed_m[static_cast<std::size_t>(i)][static_cast<std::size_t>(axis)].has_value(),
                "Submodel cases must share the constrained-node pattern");
    for(int i=0;i<count;++i)for(int axis=0;axis<3;++axis) {
        const int g=3*i+axis;
        if(cases.front().prescribed_m[static_cast<std::size_t>(i)][static_cast<std::size_t>(axis)]) {
            for(const auto& c:cases)require(std::isfinite(*c.prescribed_m[static_cast<std::size_t>(i)][static_cast<std::size_t>(axis)]),"Non-finite prescribed displacement");
        } else {freeIndex[static_cast<std::size_t>(g)]=static_cast<int>(global.size());global.push_back(g);}
    }
    const int nf=static_cast<int>(global.size());
    require(nf>0&&nf<ndof,"Submodel requires both free and constrained degrees of freedom");
    std::vector<Eigen::Triplet<double>> reduced;
    for(int col=0;col<k.outerSize();++col)for(Eigen::SparseMatrix<double>::InnerIterator it(k,col);it;++it) {
        const int rowFree=freeIndex[static_cast<std::size_t>(it.row())],colFree=freeIndex[static_cast<std::size_t>(it.col())];
        if(rowFree>=0&&colFree>=0)reduced.emplace_back(rowFree,colFree,it.value());
    }
    Eigen::SparseMatrix<double> kf(nf,nf);kf.setFromTriplets(reduced.begin(),reduced.end());
    reduced.clear();reduced.shrink_to_fit();
    Eigen::CholmodSupernodalLLT<Eigen::SparseMatrix<double>> solver;
    solver.cholmod().nmethods=1;solver.cholmod().method[0].ordering=CHOLMOD_METIS;
    solver.compute(kf);
    require(solver.info()==Eigen::Success&&solver.cholmod().status==CHOLMOD_OK,"Submodel stiffness is unstable");
    std::vector<SubmodelSolveResult> results;results.reserve(cases.size());
    for(const auto& load:cases) {
        require(load.body_acceleration_m_s2.allFinite(),"Non-finite submodel body acceleration");
        Eigen::VectorXd prescribed=Eigen::VectorXd::Zero(ndof),bodyForce=Eigen::VectorXd::Zero(ndof);
        for(int i=0;i<count;++i)for(int axis=0;axis<3;++axis) {
            const int g=3*i+axis;
            if(const auto& value=load.prescribed_m[static_cast<std::size_t>(i)][static_cast<std::size_t>(axis)])prescribed[g]=*value;
            bodyForce[g]=mass[i]*load.body_acceleration_m_s2[axis];
        }
        Eigen::VectorXd rhs(nf);const Eigen::VectorXd lifted=bodyForce-k*prescribed;
        for(int i=0;i<nf;++i)rhs[i]=lifted[global[static_cast<std::size_t>(i)]];
        const Eigen::VectorXd uf=solver.solve(rhs);
        require(solver.info()==Eigen::Success&&uf.allFinite(),"Submodel solve failed");
        Eigen::VectorXd u=prescribed;
        for(int i=0;i<nf;++i)u[global[static_cast<std::size_t>(i)]]=uf[i];
        ElasticResult result;result.volume_m3=totalVolume;
        result.free_residual_relative=(kf*uf-rhs).norm()/std::max(1.,rhs.norm());
        require(result.free_residual_relative<1e-7,"Submodel equilibrium residual exceeds tolerance");
        const Eigen::VectorXd internal=k*u,reaction=internal-bodyForce;
        Eigen::Vector3d totalLoad=Eigen::Vector3d::Zero(),totalReaction=Eigen::Vector3d::Zero();
        Eigen::Vector3d loadMoment=Eigen::Vector3d::Zero(),reactionMoment=Eigen::Vector3d::Zero();
        double fnorm=0,mnorm=0;
        for(int i=0;i<count;++i) {
            const Eigen::Vector3d v=u.segment<3>(3*i);result.displacement_m.push_back(v);
            result.max_displacement_m=std::max(result.max_displacement_m,v.norm());
            const Eigen::Vector3d f=bodyForce.segment<3>(3*i),x=mesh.nodes_mm[static_cast<std::size_t>(i)]*1e-3;
            Eigen::Vector3d r=reaction.segment<3>(3*i);
            for(int axis=0;axis<3;++axis)if(freeIndex[static_cast<std::size_t>(3*i+axis)]>=0)r[axis]=0;
            totalLoad+=f;totalReaction+=r;loadMoment+=x.cross(f);reactionMoment+=x.cross(r);
            fnorm+=f.norm();mnorm+=x.cross(f).norm();
        }
        result.total_load_n=totalLoad;result.total_reaction_n=totalReaction;
        result.total_load_moment_nm=loadMoment;result.total_reaction_moment_nm=reactionMoment;
        result.force_balance_relative=(totalLoad+totalReaction).norm()/std::max(1.,fnorm);
        result.moment_balance_relative=(loadMoment+reactionMoment).norm()/std::max(1.,mnorm);
        require(result.force_balance_relative<1e-7&&result.moment_balance_relative<1e-7,"Submodel balance failed");
        result.energy_j=0.5*u.dot(internal);
        SubmodelSolveResult solvedResult;solvedResult.field=std::move(result);
        solvedResult.stress_pa.reserve(mesh.tetrahedra.size());
        for(const auto& t:mesh.tetrahedra) {
            Eigen::Matrix4d a;
            for(int i=0;i<4;++i){a(i,0)=1;a.block<1,3>(i,1)=mesh.nodes_mm.at(static_cast<std::size_t>(t[static_cast<std::size_t>(i)])).transpose()*1e-3;}
            const Eigen::Matrix4d inverse=a.inverse();
            Eigen::Matrix<double,6,12> b=Eigen::Matrix<double,6,12>::Zero();
            for(int i=0;i<4;++i) {
                const double x=inverse(1,i),y=inverse(2,i),z=inverse(3,i);
                b(0,3*i)=x;b(1,3*i+1)=y;b(2,3*i+2)=z;
                b(3,3*i)=y;b(3,3*i+1)=x;
                b(4,3*i+1)=z;b(4,3*i+2)=y;
                b(5,3*i)=z;b(5,3*i+2)=x;
            }
            Eigen::Matrix<double,12,1> ue;
            for(int j=0;j<4;++j)ue.segment<3>(3*j)=u.segment<3>(3*t[static_cast<std::size_t>(j)]);
            const Eigen::Matrix<double,6,1> s=D*b*ue;
            const double vm=std::sqrt(0.5*(std::pow(s[0]-s[1],2)+std::pow(s[1]-s[2],2)+std::pow(s[2]-s[0],2))+
                                      3*(s[3]*s[3]+s[4]*s[4]+s[5]*s[5]));
            solvedResult.field.von_mises_pa.push_back(vm);
            solvedResult.field.max_stress_pa=std::max(solvedResult.field.max_stress_pa,vm);
            solvedResult.stress_pa.push_back(s);
        }
        results.push_back(std::move(solvedResult));
    }
    return results;
}

std::optional<double> pilkeyShoulderBendingKt(double D_mm,double d_mm,double r_mm) {
    require(std::isfinite(D_mm)&&std::isfinite(d_mm)&&std::isfinite(r_mm),"Non-finite Kt geometry");
    require(D_mm>d_mm&&d_mm>0&&r_mm>0,"Invalid stepped-shaft Kt geometry");
    const double h=(D_mm-d_mm)/2,ratio=h/r_mm;
    if(!(ratio>2&&ratio<=20))return std::nullopt;
    const double s=std::sqrt(ratio);
    const double c1=1.232+0.832*s-0.008*ratio;
    const double c2=-3.813+0.968*s-0.260*ratio;
    const double c3=7.423-4.868*s+0.869*ratio;
    const double c4=-3.839+3.070*s-0.600*ratio;
    const double g=2*h/D_mm;
    return c1+c2*g+c3*g*g+c4*g*g*g;
}

NominalSectionStress nominalBandStress(const TetMesh& mesh,const std::vector<StressTensor6>& stress_pa,
                                       double station_mm,double half_band_mm) {
    require(stress_pa.size()==mesh.tetrahedra.size(),"Nominal band stress count mismatch");
    double weightSum=0,meanAxial=0,meanVM=0;
    Eigen::Matrix3d normal=Eigen::Matrix3d::Zero();
    Eigen::Vector3d rhs=Eigen::Vector3d::Zero();
    std::vector<double> radii,shears,volumes;
    std::size_t elements=0;
    for(std::size_t e=0;e<mesh.tetrahedra.size();++e) {
        Eigen::Vector3d c=Eigen::Vector3d::Zero();
        for(int n:mesh.tetrahedra[e])c+=mesh.nodes_mm[static_cast<std::size_t>(n)]/4;
        if(std::abs(c.y()-station_mm)>half_band_mm)continue;
        const auto& t=mesh.tetrahedra[e];
        Eigen::Matrix4d a;
        for(int i=0;i<4;++i){a(i,0)=1;a.block<1,3>(i,1)=mesh.nodes_mm.at(static_cast<std::size_t>(t[static_cast<std::size_t>(i)])).transpose()*1e-3;}
        const double v=std::abs(a.determinant())/6;
        const double r=std::hypot(c.x(),c.z());
        const Eigen::Vector3d row{1,c.x(),c.z()};
        normal+=v*row*row.transpose();
        rhs+=v*row*stress_pa[e][1];
        radii.push_back(r);shears.push_back(std::hypot(stress_pa[e][3],stress_pa[e][4]));volumes.push_back(v);
        meanAxial+=v*stress_pa[e][1];
        meanVM+=v*vonMises(stress_pa[e]);
        weightSum+=v;++elements;
    }
    require(elements>=16&&weightSum>0,"Nominal stress band has insufficient elements");
    const double maxRadius=*std::max_element(radii.begin(),radii.end());
    std::vector<double> torsionScaled;
    for(std::size_t i=0;i<radii.size();++i)torsionScaled.push_back(radii[i]>1e-6?shears[i]*maxRadius/radii[i]:0);
    NominalSectionStress result;result.elements=elements;result.section_radius_mm=maxRadius;
    const Eigen::Vector3d solution=normal.ldlt().solve(rhs);
    require(solution.allFinite(),"Nominal bending fit is singular");
    result.bending_surface_pa=std::hypot(solution[1],solution[2])*maxRadius;
    result.axial_mean_pa=meanAxial/weightSum;
    result.von_mises_mean_pa=meanVM/weightSum;
    std::sort(torsionScaled.begin(),torsionScaled.end());
    result.torsion_surface_pa=torsionScaled[torsionScaled.size()/2];
    return result;
}

double capsulePerimeterDistance(const Eigen::Vector3d& p,double begin_mm,double end_mm,double radius_mm,double z0_mm) {
    require(p.allFinite(),"Non-finite capsule distance point");
    require(std::isfinite(begin_mm)&&std::isfinite(end_mm)&&end_mm>begin_mm&&std::isfinite(radius_mm)&&radius_mm>0&&
            std::isfinite(z0_mm),"Invalid key-floor capsule");
    const double dz=p.z()-z0_mm;
    const double sideY=std::clamp(p.y(),begin_mm,end_mm);
    const double side=std::sqrt(std::pow(std::abs(p.x())-radius_mm,2)+std::pow(p.y()-sideY,2)+dz*dz);
    double arc=std::numeric_limits<double>::infinity();
    for(const double end:{begin_mm,end_mm}) {
        if((end==begin_mm&&p.y()<=begin_mm)||(end==end_mm&&p.y()>=end_mm))
            arc=std::min(arc,std::sqrt(std::pow(std::hypot(p.x(),p.y()-end)-radius_mm,2)+dz*dz));
    }
    return std::min(side,arc);
}
}
