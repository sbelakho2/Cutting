#include "mesh_io.hpp"
#include <libxml/parser.h>
#include <libxml/tree.h>
#include <algorithm>
#include <charconv>
#include <cmath>
#include <fstream>
#include <iterator>
#include <memory>
#include <stdexcept>
#include <string_view>

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
    for(auto* child:children(node))if(named(child,name)){require(!found,"Duplicate VTU geometry section");found=child;}
    require(found!=nullptr,"Missing VTU geometry section");return found;
}
std::size_t count(const std::string& text,std::size_t maximum) {
    std::size_t value=0;const auto result=std::from_chars(text.data(),text.data()+text.size(),value);
    require(result.ec==std::errc{}&&result.ptr==text.data()+text.size()&&value>0&&value<=maximum,"Invalid or over-budget VTU count");return value;
}
Text arrayText(xmlNode* array,const char* type,const char* components) {
    require(named(array,"DataArray")&&property(array,"format")=="ascii"&&property(array,"type")==type,"Unsupported VTU geometry array encoding/type");
    const auto width=property(array,"NumberOfComponents");
    require(width==components||(width.empty()&&std::string_view(components)=="1"),"Wrong VTU geometry component count");
    for(auto* child=array->children;child;child=child->next)
        require(child->type==XML_TEXT_NODE||child->type==XML_CDATA_SECTION_NODE,"Nested VTU array content is unsupported");
    Text text(xmlNodeGetContent(array));require(bool(text),"Missing VTU array values");return text;
}
xmlNode* namedArray(xmlNode* node,const char* name) {
    xmlNode* found=nullptr;
    for(auto* child:children(node)) {
        require(named(child,"DataArray"),"Non-array VTU cell data");
        if(property(child,"Name")==name){require(!found,"Duplicate VTU cell array");found=child;}
    }
    require(found!=nullptr,"Missing VTU cell array");return found;
}
template<class Number,class Consumer> void numbers(const Text& text,std::size_t expected,Consumer consume) {
    const std::string_view view(reinterpret_cast<const char*>(text.get()));const char* next=view.data();const char* end=next+view.size();
    for(std::size_t i=0;i<expected;++i) {
        while(next!=end&&whitespace(*next))++next;
        Number value{};const auto result=std::from_chars(next,end,value);
        require(result.ec==std::errc{}&&result.ptr!=next&&(result.ptr==end||whitespace(*result.ptr)),"Malformed or missing VTU number");
        consume(i,value);next=result.ptr;
    }
    while(next!=end&&whitespace(*next))++next;require(next==end,"Excess VTU geometry values");
}
}
TetMesh readLinearVtuGeometry(const std::filesystem::path& path) {
    require(std::filesystem::is_regular_file(path)&&std::filesystem::file_size(path)<=256U*1024U*1024U,"Missing or oversized VTU input");
    std::ifstream file(path,std::ios::binary);require(bool(file),"VTU input cannot be opened");
    const std::string input{std::istreambuf_iterator<char>(file),std::istreambuf_iterator<char>()};require(!file.bad(),"VTU input read failed");
    require(input.find('\0')==std::string::npos&&input.find("<!DOCTYPE")==std::string::npos&&input.find("<!ENTITY")==std::string::npos,
            "VTU DTDs/entities/non-UTF8 input are unsupported");
    const auto options=XML_PARSE_NONET|XML_PARSE_HUGE|XML_PARSE_NOERROR|XML_PARSE_NOWARNING;
    const std::unique_ptr<xmlDoc,decltype(&xmlFreeDoc)> document(xmlReadMemory(input.data(),static_cast<int>(input.size()),path.c_str(),"UTF-8",options),xmlFreeDoc);
    require(bool(document)&&!document->intSubset&&!document->extSubset,"Invalid or unsupported VTU XML");
    auto* root=xmlDocGetRootElement(document.get());require(named(root,"VTKFile")&&!root->ns&&property(root,"type")=="UnstructuredGrid","Expected VTU UnstructuredGrid");
    require(property(root,"compressor").empty(),"Compressed VTU geometry is unsupported");
    auto* grid=onlyChild(root,"UnstructuredGrid");auto* piece=onlyChild(grid,"Piece");
    const auto nodeCount=count(property(piece,"NumberOfPoints"),150000),cellCount=count(property(piece,"NumberOfCells"),2000000);
    for(auto* child:children(piece))require(named(child,"Points")||named(child,"Cells")||named(child,"PointData")||named(child,"CellData"),"Unsupported VTU Piece section");
    auto* points=uniqueChild(piece,"Points");auto* cells=uniqueChild(piece,"Cells");
    require(children(cells).size()==3,"Expected exactly three VTU connectivity arrays");
    TetMesh mesh;mesh.nodes_mm.resize(nodeCount);mesh.tetrahedra.resize(cellCount);
    const auto coordinates=arrayText(onlyChild(points,"DataArray"),"Float64","3");
    numbers<double>(coordinates,3*nodeCount,[&](std::size_t i,double value){require(std::isfinite(value),"Non-finite VTU coordinate");mesh.nodes_mm[i/3][static_cast<int>(i%3)]=value;});
    const auto connectivity=arrayText(namedArray(cells,"connectivity"),"Int32","1");
    numbers<int>(connectivity,4*cellCount,[&](std::size_t i,int value){require(value>=0&&static_cast<std::size_t>(value)<nodeCount,"VTU node index out of range");mesh.tetrahedra[i/4][i%4]=value;});
    const auto offsets=arrayText(namedArray(cells,"offsets"),"Int32","1");
    numbers<int>(offsets,cellCount,[&](std::size_t i,int value){require(value>=0&&static_cast<std::size_t>(value)==4*(i+1),"VTU tetrahedron offset mismatch");});
    const auto types=arrayText(namedArray(cells,"types"),"UInt8","1");
    numbers<int>(types,cellCount,[](std::size_t,int value){require(value==10,"Expected linear four-node VTK tetrahedra");});
    for(const auto& cell:mesh.tetrahedra){auto sorted=cell;std::sort(sorted.begin(),sorted.end());require(std::adjacent_find(sorted.begin(),sorted.end())==sorted.end(),"VTU cell repeats a vertex");}
    return mesh;
}
}