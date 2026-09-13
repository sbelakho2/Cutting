#include "shared/cutting_head_spec.hpp"
#include "shared/feed_drive_spec.hpp"
#include "shared/measurement_spec.hpp"
#include <QGuiApplication>
#include <QPainter>
#include <QPdfWriter>
#include <QPageSize>
#include <QSvgRenderer>
#include <array>
#include <cmath>
#include <filesystem>
#include <fstream>
#include <iomanip>
#include <iostream>
#include <numbers>
#include <sstream>
#include <stdexcept>

namespace fs=std::filesystem;
namespace d=starz::head::design;
namespace c=starz::head::catalogue;
namespace {
void text(std::ostream& s,double x,double y,const std::string& value,double size=3.1) {
    s<<"<text x='"<<x<<"' y='"<<y<<"' font-size='"<<size<<"'>"<<value<<"</text>\n";
}
void line(std::ostream& s,double x1,double y1,double x2,double y2,const char* css="edge") {
    s<<"<line class='"<<css<<"' x1='"<<x1<<"' y1='"<<y1<<"' x2='"<<x2<<"' y2='"<<y2<<"'/>\n";
}
void circle(std::ostream& s,double x,double y,double r,const char* css="edge") {
    s<<"<circle class='"<<css<<"' cx='"<<x<<"' cy='"<<y<<"' r='"<<r<<"'/>\n";
}
void dimension(std::ostream& s,double a,double b,double base,double at,const std::string& label) {
    line(s,a,base,a,at+2,"dim");line(s,b,base,b,at+2,"dim");
    s<<"<path class='dim' marker-start='url(#arrow)' marker-end='url(#arrow)' d='M"<<a<<' '<<at<<" H"<<b<<"'/>\n";
    text(s,(a+b)/2-12,at-1.4,label);
}
void begin(std::ostream& s,const std::string& title="H003 — CUTTING MOTOR ADAPTER / R6") {
    s<<"<svg xmlns='http://www.w3.org/2000/svg' width='420mm' height='297mm' viewBox='0 0 420 297'>\n"
      <<"<defs><marker id='arrow' markerWidth='5' markerHeight='5' refX='2.5' refY='2.5' orient='auto-start-reverse'><path d='M5 0 L0 2.5 L5 5' fill='none' stroke='#111' stroke-width='.5'/></marker></defs>\n"
      <<"<style>text{font-family:DejaVu Sans,Arial,sans-serif;fill:#111}.edge{fill:none;stroke:#111;stroke-width:.35}.dim{fill:none;stroke:#444;stroke-width:.18}.centre{fill:none;stroke:#888;stroke-width:.16;stroke-dasharray:4 1 1 1}</style>\n"
      <<"<rect width='420' height='297' fill='white'/><rect class='edge' x='8' y='8' width='404' height='281'/>\n";
    text(s,14,20,"STARZ ELECTRONICS",7);
    text(s,14,28,title,4.5);
    text(s,250,18,"ENGINEERING DRAWING — RELEASE HOLD",4);
    text(s,250,25,"Units mm · A3 · Do not scale dimensions",3);
}
void adapter(const fs::path& out) {
    const auto path=out/"H003-R6-adapter.svg";std::ofstream s(path);
    s<<std::setprecision(10);begin(s);
    const double x=85,y=105,q=35/std::sqrt(2.0);
    s<<"<rect class='edge' x='35' y='55' width='100' height='100'/>\n";
    circle(s,x,y,21);circle(s,x,y,d::adapter_motor_pilot_diameter/2);
    circle(s,x,y,d::adapter_f0_register_diameter/2,"centre");
    circle(s,x,y,35,"centre");line(s,27,y,143,y,"centre");line(s,x,47,x,163,"centre");
    for(double a:{-q,q})for(double b:{-q,q}){circle(s,x+a,y+b,2.75);circle(s,x+a,y+b,4.75);}
    for(double a:{-c::motor_pitch/2,c::motor_pitch/2})for(double b:{-c::motor_pitch/2,c::motor_pitch/2})circle(s,x+a,y+b,2.1);
    dimension(s,35,135,155,168,"100.00 ±0.05");
    dimension(s,x-c::motor_pitch/2,x+c::motor_pitch/2,55,45,"69.58 BASIC");
    text(s,39,180,"MOTOR-SIDE VIEW / 1:1",3.5);
    text(s,17,190,"4 × M5×0.8-6H THRU; square69.58 BASIC; position Ø0.10 to A|B|C");
    text(s,17,197,"4 × Ø5.50 +0.10/0 THRU on PCD70 BASIC,45° quadrants");
    text(s,17,204,"Counterbore Ø9.50 +0.10/0 ×8.50 ±0.05 deep FROM MOTOR FACE");
    text(s,17,211,"Pilot pocket Ø73.10 +0.02/0 ×2.10 ±0.02 deep;central Ø42.00 +0.10/0");

    // Axial section: motor face left, F0 face right. Radial coordinate shown
    // above/below centreline. Draw the actual15mm plate and2mm F0 boss.
    const double sx=210,cy=105,t=d::adapter_thickness;
    const auto step=[&](double r1,double r2,double depth) {
        s<<"<path class='edge' d='M"<<sx<<' '<<cy-r2<<" H"<<sx+depth<<" V"<<cy-r1<<" H"<<sx+t<<" M"<<sx<<' '<<cy+r2<<" H"<<sx+depth<<" V"<<cy+r1<<" H"<<sx+t<<"'/>\n";
    };
    line(s,sx,55,sx,cy-d::adapter_motor_pilot_diameter/2);line(s,sx,cy+d::adapter_motor_pilot_diameter/2,sx,155);
    line(s,sx,55,sx+t,55);line(s,sx,155,sx+t,155);
    line(s,sx+t,55,sx+t,cy-d::adapter_f0_register_diameter/2);
    line(s,sx+t,cy+d::adapter_f0_register_diameter/2,sx+t,155);
    step(21,d::adapter_motor_pilot_diameter/2,d::adapter_motor_pilot_depth);
    s<<"<path class='edge' d='M"<<sx+t<<' '<<cy-d::adapter_f0_register_diameter/2<<" H"<<sx+t+2<<" V"<<cy-21<<" H"<<sx+t
      <<" M"<<sx+t<<' '<<cy+21<<" H"<<sx+t+2<<" V"<<cy+d::adapter_f0_register_diameter/2<<" H"<<sx+t<<"'/>\n";
    line(s,197,cy,245,cy,"centre");dimension(s,sx,sx+t,155,168,"15.00 ±0.02");
    text(s,193,180,"AXIAL SECTION / 1:1",3.5);
    text(s,258,57,"DATUM A: F0 mounting face");
    text(s,258,64,"DATUM B: Ø49.97 F0 register axis");
    text(s,258,71,"DATUM C: left edge X−50 in this view");
    text(s,258,82,"F0 boss: Ø49.97 ±0.01 ×2.00 ±0.02");
    text(s,258,89,"A flatness0.01;motor face parallel0.01 to A");
    text(s,258,96,"Pilot pocket position Ø0.03 to A|B");
    text(s,258,103,"Mounting/pilot surfaces Ra0.8µm");
    text(s,258,110,"Other machined surfaces Ra3.2µm");
    text(s,258,121,"Material42CrMo4+QT,min650MPa yield");
    text(s,258,128,"Rough stock105×105×20;quantity2");
    text(s,258,135,"No paint/plating on locating or joint faces");
    text(s,258,146,"General edges0.2×45°;remove all burrs");
    text(s,258,153,"Do not countersink screw bearing seats");
    text(s,258,166,"F0:4 ×ISO4762 M5×16,class10.9");
    text(s,258,173,"withISO7092 M5 washers Ø9×1,200HV");
    text(s,258,180,"Motor:4 ×M5×25,class10.9,same washers");
    text(s,258,191,"Counterbore depth clears the motor pilot");
    text(s,258,198,"Old5.5mm recess FAILS screw-head collision");
    text(s,258,209,"Joint preload and peak convergence open");
    line(s,8,222,412,222);
    text(s,14,231,"PROCESS: rough all features;finish A and motor face in controlled setups;finish concentric registers;drill/tap;deburr;inspect.");
    text(s,14,239,"INSPECT: thickness at4corners;A/face flatness;both register diameters;pilot depth;counterbore depths;hole position;M5thread gauges.");
    text(s,14,247,"ASSEMBLY: remove motor key;use actual free-length coupling;set stage hub1.00±0.05from locknut shoulder;do not stretch bellows.");
    text(s,14,255,"Release hold: updated adapter stress/convergence,joint/thread checks,and remaining full-machine integration. Drawing is not a release stamp.");
    line(s,8,264,412,264);line(s,235,264,235,289);
    text(s,14,274,"PART H003 · ADAPTER · SINGLE SELECTED DESIGN",4);
    text(s,14,282,starz::head::revision,3.2);
    text(s,241,274,"SCALE1:1 / THIRD-ANGLE FRONT + AXIAL SECTION",3);
    text(s,241,282,"SOURCE: shared cutting_head_spec and catalogue evidence",3);
    s<<"</svg>\n";s.close();if(!s)throw std::runtime_error("Drawing SVG output failed");
    QSvgRenderer renderer(QString::fromStdString(path.string()));
    if(!renderer.isValid())throw std::runtime_error("Generated drawing SVG invalid");
    QPdfWriter pdf(QString::fromStdString((out/"H003-R6-adapter.pdf").string()));
    pdf.setPageSize(QPageSize(QPageSize::A3));pdf.setPageOrientation(QPageLayout::Landscape);
    pdf.setPageMargins(QMarginsF(0,0,0,0));pdf.setResolution(300);
    pdf.setTitle("STARZ H003 R6 adapter — engineering drawing,release hold");
    QPainter painter(&pdf);if(!painter.isActive())throw std::runtime_error("Drawing PDF output failed");
    renderer.render(&painter,QRectF(0,0,pdf.width(),pdf.height()));painter.end();
    // Inline SVG inside an HTML document also makes browser review accessible;
    // standalone SVG documents have no body element in some editor webviews.
    std::ifstream drawing(path);std::ofstream review(out/"index.html");
    review<<"<!doctype html><html lang='en'><meta charset='utf-8'><title>STARZ H003 drawing review</title>"
          <<"<style>body{margin:24px;background:#e5e7eb;font-family:Arial,sans-serif;color:#0b0f14}"
          <<"header{display:flex;gap:24px;align-items:center;margin-bottom:16px}h1{font-size:20px}"
          <<"a{color:#b00020}svg{display:block;width:100%;height:auto;background:white;box-shadow:0 2px 12px #0002}</style>"
          <<"<body><header><h1>STARZ · H003 R6 adapter</h1><a href='H003-R6-adapter.pdf'>A3 vector PDF</a>"
          <<"<span>Engineering drawing · release hold</span></header>"<<drawing.rdbuf()<<"</body></html>";
    if(!drawing||!review)throw std::runtime_error("Drawing review output failed");
}
void shaft(const fs::path& out) {
    const auto path=out/"F101-R6-shaft.svg";std::ofstream s(path);
    s<<std::setprecision(10);begin(s,"F101 — FEED ROTOR SHAFT / R6 · SHEET1/2");
    // Orthographic side profile,2:1 onA3;S increases to the right,+Z upwards.
    constexpr double x0=25,y0=81,k=2;
    const std::array<std::array<double,3>,9> turns{{{0,18.5,12},{18.5,19.62,11.45},{19.62,31.5,12},
        {31.5,64.5,14},{64.5,71.5,15},{71.5,73,13.5},{73,82,15},{82,87,19},{87,92,12}}};
    double last_r=0;
    for(const auto& q:turns) {
        const double a=x0+k*q[0],b=x0+k*q[1],r=k*q[2]/2;
        const double lead=(q[0]==0?0.2:(q[0]==64.5?0.5:0));
        for(int sign:{-1,1}) {
            line(s,a+k*lead,y0+sign*r,b,y0+sign*r);
            if(lead>0) {
                line(s,a,y0+sign*(r-k*lead),a+k*lead,y0+sign*r);
                line(s,a,y0+sign*last_r,a,y0+sign*(r-k*lead));
            }else line(s,a,y0+sign*last_r,a,y0+sign*r);
        }
        last_r=r;
    }
    line(s,x0+k*92,y0-last_r,x0+k*92,y0-10);line(s,x0+k*92,y0+last_r,x0+k*92,y0+10);
    for(int sign:{-1,1}) {
        line(s,x0+k*92,y0+sign*10,x0+k*112.8,y0+sign*10);
        line(s,x0+k*112.8,y0+sign*10,x0+k*113,y0+sign*9.6);
    }
    line(s,x0+k*113,y0-9.6,x0+k*113,y0+9.6);line(s,18,y0,260,y0,"centre");
    // Key-seat floor and drilled-hole detail;small radii governed by callouts.
    line(s,x0+k*33.45,y0-8,x0+k*61.55,y0-8);
    line(s,x0+k*94.45,y0+6.4,x0+k*112.55,y0+6.4);
    line(s,x0+k*31.2,y0-11,x0+k*73,y0-11,"centre");
    for(int sign:{-1,1}) {
        line(s,x0+k*95,y0+sign*4.2,x0+k*98.8,y0+sign*4.2,"centre");
        line(s,x0+k*98.8,y0+sign*5,x0+k*113,y0+sign*5,"centre");
        line(s,x0+k*95,y0+sign*4.2,x0+k*(95-2.1/std::tan(59*std::numbers::pi/180)),y0,"centre");
    }
    dimension(s,x0,x0+k*113,y0+19,114,"113.00 ±0.02");
    text(s,23,126,"SIDE VIEW / 2:1 · S from rear end · small root radii per callouts",3);
    text(s,275,46,"S interval       Diameter / thread",3.4);
    const std::array<std::string,10> sizes{{"0–18.5           Ø12 h6","18.5–19.62    groove Ø11.45 ±0.02",
        "19.62–31.5    Ø12 matched j5","31.5–64.5      Ø14 h6","64.5–71.5      M15×1-6g","71.5–73         relief Ø13.50 ±0.02",
        "73–82             Ø15 matched j5","82–87             Ø19 ±0.02","87–92             Ø12 −0.02/0","92–113           Ø10 h6"}};
    for(std::size_t i=0;i<sizes.size();++i)text(s,275,54+7*static_cast<double>(i),sizes[i]);
    text(s,17,139,"DATUM A: common bearing-seat axis. DATUM B: front bearing shoulder S82.",3.4);
    text(s,17,147,"Bearing seats: j5 −0.003/+0.005 AND 0.003–0.006 actual interference to assigned SKF P6 bore.");
    text(s,17,154,"12 h6 /14 h6: −0.011/0;10 h6: −0.009/0. Match-bearing table governs final seat diameters.");
    text(s,17,161,"Bearing-seat cylindricity0.001,total radial runout0.002 to A;B perpendicularity0.004 to A.");
    text(s,17,168,"Ø14/Ø10 total runout0.010 to A. Bearing seats Ra0.4µm;roller/coupling0.8µm;other surfaces3.2µm.");
    text(s,17,177,"PULLEY KEY: S33.45–61.55,L28.10±0.02,W5.04±0.01,on+Z;floorZ4.00±0.02;endsR2.52.",2.8);
    text(s,17,183,"ROLLER KEY: S94.45–112.55,L18.10±0.02,W3.04±0.01,on−Z;floorZ−3.20±0.02;endsR1.52.",2.8);
    text(s,17,189,"Both key-floor perimeters R0.15±0.02;matching keys:shaft-facing perimeter0.20±0.02×45°.",2.8);
    text(s,17,195,"MB2 SLOT: S31.20–73.00,W4.20±0.05,on+Z;floorZ5.50±0.05;R0.50plan/intersections,R0.15floor. See sheet2.",2.6);
    text(s,17,201,"GROOVE: S18.50±0.02,W1.12±0.02,Ø11.45±0.02;both rootsR0.04±0.01;SEEGER A12.",2.8);
    text(s,17,207,"SHOULDERS: S31.5/S82/S92 R0.20±0.02;S87 R0.50±0.05;M15 relief rootsR0.20±0.02.",2.8);
    text(s,17,213,"END TAP: M5×0.8-6H,14.00–14.05full thread AFTER C0.20±0.02;drillØ4.20+0.03/0×18.00±0.05 PLUS118°±2° point.",2.6);
    line(s,8,218,412,218);
    text(s,17,228,"Material42CrMo4+QT,certified yield≥650MPa;rough stockØ20×120;quantity8. General axial coordinates±0.02.");
    text(s,17,236,"ENDS: external C0.20±0.02 at S0/S113;M15 lead C0.50±0.02. Full-profile M15 thread required over S65.10–71.30.",2.8);
    text(s,17,244,"ASSEMBLY: fit MB2 BEFORE pulley key. Keep F112 removable;insert preassembled rotor from+S with locating dowels removed.");
    text(s,17,252,"RELEASE HOLD: local stress/contact,thermal-field validation and joint preload. R6 edge geometry is defined on both sheets.");
    line(s,8,264,412,264);line(s,235,264,235,289);
    text(s,14,274,"PART F101 · FEED ROTOR · SHAFT",4);
    text(s,14,282,std::string(starz::feed::revision),3.2);
    text(s,241,274,"SCALE2:1 / DIMENSION RECORD / NO RELEASE STAMP",3);
    text(s,241,282,"Companion: feed cartridge assembly and bearing matching record",2.8);
    s<<"</svg>\n";s.close();if(!s)throw std::runtime_error("Shaft SVG output failed");
    QSvgRenderer renderer(QString::fromStdString(path.string()));if(!renderer.isValid())throw std::runtime_error("Shaft SVG invalid");
    QPdfWriter pdf(QString::fromStdString((out/"F101-R6-shaft.pdf").string()));
    pdf.setPageSize(QPageSize(QPageSize::A3));pdf.setPageOrientation(QPageLayout::Landscape);pdf.setPageMargins(QMarginsF(0,0,0,0));pdf.setResolution(300);
    pdf.setTitle("STARZ F101 R6 shaft — sheet1 of2,release hold");
    QPainter painter(&pdf);if(!painter.isActive())throw std::runtime_error("Shaft PDF failed");
    renderer.render(&painter,QRectF(0,0,pdf.width(),pdf.height()));painter.end();
    std::ifstream drawing(path);std::ofstream review(out/"shaft-review.html");
    review<<"<!doctype html><html lang='en'><meta charset='utf-8'><title>STARZ F101 drawing review</title>"
          <<"<style>body{margin:24px;background:#e5e7eb;font-family:Arial,sans-serif}svg{width:100%;height:auto;background:white}</style>"
          <<"<body><h1>F101 R6 shaft dimension record</h1><p><a href='F101-R6-shaft.pdf'>A3 sheet1 PDF</a> · <a href='F101-R6-details.pdf'>A3 sheet2 PDF</a> · <a href='shaft-details-review.html'>Machining detail review</a> · Engineering release hold</p>"
          <<drawing.rdbuf()<<"</body></html>";
    if(!drawing||!review)throw std::runtime_error("Shaft review output failed");
}
void shaftDetails(const fs::path& out) {
        namespace f=starz::feed::design;
        const auto path=out/"F101-R6-details.svg";std::ofstream s(path);
        s<<std::setprecision(10);begin(s,"F101 — SLOT / END MACHINING DETAILS · R6 · SHEET2/2");
        // Analytic constant-height outline of the two-pocket union at Z6.2.
        // The R.5 intersection centres are the intersection of the offset straight
        // wall and the key-end circle offset by the same radius; no polyline fit.
        constexpr double k=5,x0=24,y0=80,keyR=2.52,keyNear=35.97,keyFar=59.03;
        const double half=f::washer_slot_width/2,r=f::washer_slot_intersection_radius;
        const double off=std::sqrt(std::pow(keyR+r,2)-std::pow(half+r,2));
        const double near=keyNear-off,far=keyFar+off;
        const double contactX=(half+r)*keyR/(keyR+r),contactNear=keyNear-off*keyR/(keyR+r),contactFar=keyFar+off*keyR/(keyR+r);
        const auto px=[&](double station){return x0+k*(station-30);};
        const auto py=[&](double transverse){return y0-k*transverse;};
        s<<"<path class='edge' d='M"<<px(31.7)<<' '<<py(half);
        const auto go=[&](double station,double x){s<<" L"<<px(station)<<' '<<py(x);};
        const auto arc=[&](double radius,int sweep,double station,double x){s<<" A"<<k*radius<<' '<<k*radius<<" 0 0 "<<sweep<<' '<<px(station)<<' '<<py(x);};
        go(near,half);arc(r,0,contactNear,contactX);arc(keyR,1,keyNear,keyR);go(keyFar,keyR);
        arc(keyR,1,contactFar,contactX);arc(r,0,far,half);go(72.5,half);arc(0.5,1,73,1.6);go(73,-1.6);arc(0.5,1,72.5,-half);
        go(far,-half);arc(r,0,contactFar,-contactX);arc(keyR,1,keyFar,-keyR);go(keyNear,-keyR);
        arc(keyR,1,contactNear,-contactX);arc(r,0,near,-half);go(31.7,-half);arc(0.5,1,31.2,-1.6);go(31.2,1.6);arc(0.5,1,31.7,half);
        s<<" Z'/>\n";
        line(s,px(30.5),y0,px(74),y0,"centre");
        for(double station:{31.2,33.45,61.55,73.0})line(s,px(station),102,px(station),116,"dim");
        text(s,px(31.2)-9,124,"S31.20",2.8);text(s,px(33.45)-3,115,"S33.45",2.8);
        text(s,px(61.55)-9,115,"S61.55",2.8);text(s,px(73)-9,124,"S73.00",2.8);
        text(s,24,47,"POCKET TOOL SECTION Z6.20 / 5:1 · S increases right",3.1);
        text(s,24,137,"Near/far intersection coordinates are REF;nominal key circles R2.52,centres S35.97/59.03.",2.7);
        text(s,268,48,"MB2 slot: W4.20±0.05",3.1);
        text(s,268,56,"Start S31.20±0.02;end S73.00±0.02");
        text(s,268,64,"Four plan corners R0.50±0.02");
        text(s,268,72,"Four vertical key intersections R0.50±0.02");
        text(s,268,80,"All slot floor/lip blends R0.15±0.02");
        text(s,268,88,"Floor Z5.50±0.05;key floor Z4.00±0.02");
        text(s,268,100,"Do not round the key's straight load flanks");
        text(s,268,108,"End0.00–0.04 before actual Ø15 seat start");
        text(s,268,116,"Fit MB2 before the pulley key");
        text(s,268,124,"Rear entry is interrupted by the Ø12 land");
        line(s,8,145,412,145);
        // Enlarged local slot-floor section. The cylinder crown is deliberately
        // outside this local detail; the floor and side-wall radii are dimensional.
        constexpr double cx=74,base=192,q=10;
        s<<"<path class='edge' d='M"<<cx-q*half<<" 163 V"<<base-q*0.15
            <<" A1.5 1.5 0 0 0 "<<cx-q*(half-0.15)<<' '<<base<<" H"<<cx+q*(half-0.15)
            <<" A1.5 1.5 0 0 0 "<<cx+q*half<<' '<<base-q*0.15<<" V163'/>\n";
        line(s,36,base,113,base,"centre");
        dimension(s,cx-q*half,cx+q*half,163,156,"4.20 ±0.05");
        text(s,28,203,"S70 LOCAL FLOOR SECTION / 10:1",3);
        text(s,28,211,"Z5.50±0.05 · R0.15±0.02 both roots",3);
        text(s,144,156,"END / THREAD / FINISH REQUIREMENTS",3.4);
        text(s,144,166,"S0 and S113 external edges: 0.20±0.02×45°.");
        text(s,144,175,"M15 lead at S64.50: 0.50±0.02×45°;full profile S65.10–71.30.");
        text(s,144,184,"M5 entry: C0.20±0.02,independent of the external end chamfer.");
        text(s,144,193,"Final slot/key machining precedes matched journal finishing.");
        text(s,144,202,"Remove raised lips;edge break0.05–0.10 on remaining slot openings.");
        text(s,144,211,"Do not touch bearing-seat lands or dimensional key load flanks.");
        line(s,8,220,412,220);
        text(s,14,231,"INSPECT: optical profile/replica of slot roots and intersections;slot start/end/width/depth;end chamfers;thread GO/NO-GO and full depth.",2.8);
        text(s,14,239,"SECTION AUDIT:191 nominal B-rep sections compared against the reduced beam I;this does not resolve local peak stress or fatigue.",2.8);
        text(s,14,247,"Angular C:pulley-key radial midplane,+Z toward opening. Roller key180°±0.10°;MB2 slot median offset≤0.025 to C.",2.8);
        text(s,14,255,"RELEASE HOLD: local stress/contact,joint preload and operating thermal field. No machining or physical inspection result is prefilled.",2.8);
        line(s,8,264,412,264);line(s,235,264,235,289);
        text(s,14,274,"PART F101 · FEED ROTOR · DETAIL SHEET",4);
        text(s,14,282,std::string(starz::feed::revision),3.2);
        text(s,241,274,"SCALES5:1 /10:1 · SHEET2 OF2",3);
        text(s,241,282,"All unspecified axial coordinates ±0.02 · do not scale",3);
        s<<"</svg>\n";s.close();if(!s)throw std::runtime_error("Shaft detail SVG output failed");
        QSvgRenderer renderer(QString::fromStdString(path.string()));if(!renderer.isValid())throw std::runtime_error("Shaft detail SVG invalid");
        QPdfWriter pdf(QString::fromStdString((out/"F101-R6-details.pdf").string()));
        pdf.setPageSize(QPageSize(QPageSize::A3));pdf.setPageOrientation(QPageLayout::Landscape);pdf.setPageMargins(QMarginsF(0,0,0,0));pdf.setResolution(300);
        pdf.setTitle("STARZ F101 R6 machining details — sheet2 of2,release hold");
        QPainter painter(&pdf);if(!painter.isActive())throw std::runtime_error("Shaft detail PDF failed");
        renderer.render(&painter,QRectF(0,0,pdf.width(),pdf.height()));painter.end();
        std::ifstream drawing(path);std::ofstream review(out/"shaft-details-review.html");
        review<<"<!doctype html><html lang='en'><meta charset='utf-8'><title>STARZ F101 machining details</title>"
                    <<"<style>body{margin:24px;background:#e5e7eb;font-family:Arial,sans-serif}svg{width:100%;height:auto;background:white}</style>"
                    <<"<body><h1>F101 R6 machining details</h1><p><a href='F101-R6-details.pdf'>A3 sheet2 PDF</a> · <a href='shaft-review.html'>Sheet1</a></p>"
                    <<drawing.rdbuf()<<"</body></html>";
        if(!drawing||!review)throw std::runtime_error("Shaft detail review output failed");
}
void encoderPlate(const fs::path& out) {
    namespace m=starz::measurement;
    const auto path=out/"M201-R1-encoder-plate.svg";std::ofstream s(path);
    s<<std::setprecision(10);begin(s,"M201 — MEASURING ENCODER PLATE / R1");
    constexpr double cx=85,cy=100,scale=2;
    const double width=m::design::encoder_plate_width_mm;
    s<<"<rect class='edge' x='"<<cx-width<<"' y='"<<cy-width<<"' width='"<<2*width<<"' height='"<<2*width<<"'/>\n";
    circle(s,cx,cy,m::design::encoder_plate_pilot_mm);
    circle(s,cx,cy,m::catalogue::encoder_mount_pcd_mm,"centre");
    line(s,31,cy,139,cy,"centre");line(s,cx,46,cx,154,"centre");
    for(int i=0;i<3;++i) {
        const double angle=i*2*std::numbers::pi/3;
        circle(s,cx+30*std::sin(angle),cy-30*std::cos(angle),m::design::encoder_plate_encoder_clearance_mm);
    }
    for(double x:{-36.0,36.0})for(double z:{-36.0,36.0})circle(s,cx+x,cy-z,m::design::encoder_plate_support_clearance_mm);
    dimension(s,cx-width,cx+width,cy+width,164,"46.00 ±0.05");
    dimension(s,cx-36,cx+36,54,43,"36 BASIC");
    text(s,37,176,"VIEW FROM COUPLING SIDE / 2:1",3.3);
    text(s,17,189,"3 × Ø3.40 +0.10/0 THRU;PCD30 BASIC;120° apart;one hole on+Z.");
    text(s,17,197,"4 × Ø4.50 +0.10/0 THRU;36×36 BASIC square;future support attachment.");
    text(s,17,205,"Central bore Ø20.040 +0.020/0 THRU;no shoulder against encoder pilot tip.");
    text(s,17,213,"All hole positions Ø0.10 to A|B|C. Hole coordinates below are BASIC.");
    // Axial section: face A at left;pilot protrudes5 of the6mm plate thickness.
    constexpr double sx=205,sy=100;
    const double t=scale*m::design::encoder_plate_thickness_mm,r=m::design::encoder_plate_pilot_mm;
    for(int sign:{-1,1})s<<"<rect class='edge' x='"<<sx<<"' y='"<<(sign<0?sy-width:sy+r)<<"' width='"<<t<<"' height='"<<width-r<<"'/>\n";
    line(s,sx-10,sy,sx+25,sy,"centre");dimension(s,sx,sx+t,146,164,"6.00 ±0.02");
    text(s,188,176,"AXIAL SECTION / 2:1",3.3);
    text(s,258,46,"DATUM A: encoder mounting face Y0");
    text(s,258,53,"DATUM B: central bore axis");
    text(s,258,60,"DATUM C: left edge X−23 in this view");
    text(s,258,71,"A flatness0.02;opposite face parallel0.02");
    text(s,258,78,"B perpendicularity0.02 to A");
    text(s,258,85,"B cylindricity0.01;pilot bore Ra0.8µm");
    text(s,258,92,"Joint faces Ra1.6µm;other faces Ra3.2µm");
    text(s,258,103,"Material EN AW-6082-T6;no coating on A/B");
    text(s,258,110,"Stock50×50×8;quantity2;edgesC0.20±0.05");
    text(s,258,121,"Encoder holes (X,Z),PCD30 BASIC:");
    text(s,258,128,"(0,+15), (+12.990381,−7.5)");
    text(s,258,135,"(−12.990381,−7.5)");
    text(s,258,146,"3 ×ISO4762 M3×10,class8.8");
    text(s,258,153,"3 ×ISO7092 M3,3.2×6×0.5,200HV");
    text(s,258,164,"Nominal encoder thread engagement3.5");
    text(s,258,171,"Nominal thread-bottom clearance1.5");
    text(s,258,182,"Encoder pilot19.98–20.00 ×5 long");
    text(s,258,189,"Diametral pilot clearance0.040–0.080");
    text(s,258,200,"Encoder tip20 from A,not15");
    text(s,258,207,"Final mounting torque/support still open");
    line(s,8,222,412,222);
    text(s,14,231,"PROCESS: square stock;finish A/opposite face;finish pilot bore;drill3PCD holes and4support holes;deburr without dishing A.");
    text(s,14,239,"INSPECT:46square;6thickness;pilot diameter/cylindricity;face flatness/parallelism;hole positions and clearance diameters.");
    text(s,14,247,"ASSEMBLY: encoder body behind A;pilot passes freely;washers on coupling side. Do not use screws to pull a misaligned pilot in.");
    text(s,14,255,"SCOPE: eight-part encoder face mount only. Pressure arm,shaft bearings,complete tool access and wire guides are not this drawing.");
    line(s,8,264,412,264);line(s,235,264,235,289);
    text(s,14,274,"PART M201 · MEASURING ENCODER PLATE",4);
    text(s,14,282,std::string(m::revision),3.2);
    text(s,241,274,"SCALE2:1 / SINGLE SELECTED DIMENSION RECORD",3);
    text(s,241,282,"SOURCE: Autonics E40 drawing;STARZ measurement_spec",3);
    s<<"</svg>\n";s.close();if(!s)throw std::runtime_error("Encoder plate SVG failed");
    QSvgRenderer renderer(QString::fromStdString(path.string()));if(!renderer.isValid())throw std::runtime_error("Encoder plate SVG invalid");
    QPdfWriter pdf(QString::fromStdString((out/"M201-R1-encoder-plate.pdf").string()));
    pdf.setPageSize(QPageSize(QPageSize::A3));pdf.setPageOrientation(QPageLayout::Landscape);pdf.setPageMargins(QMarginsF(0,0,0,0));pdf.setResolution(300);
    pdf.setTitle("STARZ M201 R1 encoder plate — dimension record,release hold");
    QPainter painter(&pdf);if(!painter.isActive())throw std::runtime_error("Encoder plate PDF failed");
    renderer.render(&painter,QRectF(0,0,pdf.width(),pdf.height()));painter.end();
    std::ifstream drawing(path);std::ofstream review(out/"encoder-plate-review.html");
    review<<"<!doctype html><html lang='en'><meta charset='utf-8'><title>STARZ M201 drawing review</title>"
          <<"<style>body{margin:24px;background:#e5e7eb;font-family:Arial,sans-serif}svg{width:100%;height:auto;background:white}</style>"
          <<"<body><h1>M201 R1 encoder plate</h1><p><a href='M201-R1-encoder-plate.pdf'>A3 vector PDF</a> · Engineering release hold</p>"
          <<drawing.rdbuf()<<"</body></html>";
    if(!drawing||!review)throw std::runtime_error("Encoder plate review output failed");
}
}
int main(int argc,char** argv) try {
    QGuiApplication app(argc,argv);
    if(argc!=3||std::string(argv[1])!="--out")throw std::invalid_argument("Use --out output_directory");
    const fs::path out=argv[2];fs::create_directories(out);adapter(out);shaft(out);shaftDetails(out);encoderPlate(out);
    std::cout<<"H003 R6,F101 R6(two sheets) andM201 R1 A3 dimensioned SVG/vector PDFs generated\n";
    return 0;
}catch(const std::exception& e){std::cerr<<e.what()<<'\n';return 1;}