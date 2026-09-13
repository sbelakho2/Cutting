// Finishing-card arithmetic regression. All records here are SYNTHETIC test
// fixtures; no physical bearing or shaft has been measured.
#include "finishing.hpp"
#include <cmath>
#include <iostream>
#include <stdexcept>

namespace fin=starz::engineering::finishing;
namespace {
void require(bool ok,const char* what){if(!ok)throw std::runtime_error(what);}
bool near(double a,double b,double tol=1e-9){return std::abs(a-b)<=tol;}
template<class F> void rejects(F&& f,const char* what){try{f();}catch(const std::exception&){return;}throw std::runtime_error(what);}
}
int main()try {
    // Temperature reduction: 12mm steel at 23°C reads 12mm·0.012·3=0.432µm large.
    require(near(fin::toReference(12.0,23.0,fin::alpha_steel_um_mm_k),12.0-0.000432),"steel temperature correction");
    require(near(fin::toReference(28.013,20.0,fin::alpha_6082_um_mm_k),28.013),"no correction at 20°C");
    rejects([]{fin::toReference(12,31,fin::alpha_steel_um_mm_k);},"out-of-range temperature accepted");

    // Synthetic P6 bearing bore 11.996 at 20°C: j5 zone 11.997–12.005 ∩ (bore+3µm…bore+6µm)=11.999–12.002.
    fin::SeatRecord r{"SYN-SHAFT-1","S22.5-30.5_d12",12,"SYN-6001-A",11.996,20.0,"SYN-GAUGE-1",0.5};
    const auto c=fin::grindCard(r);
    require(near(c.accept_lower_mm,11.999)&&near(c.accept_upper_mm,12.002),"j5 intersection");
    require(near(c.guarded_lower_mm,11.9995)&&near(c.guarded_upper_mm,12.0015),"guard band 0.5µm");
    require(near(c.target_mm,12.0005),"target mid guarded window");
    require(near(c.pre_grind_lower_mm,12.25)&&near(c.pre_grind_upper_mm,12.35),"pre-grind limits");
    // stock from 12.35 to 12.0005 = 0.3495; rough to 0.040 → ceil(0.3095/0.020)=16; finish ceil(0.030/0.005)=6; final ceil(0.010/0.002)=5.
    require(c.rough_passes==16&&c.finish_passes==6&&c.final_passes==5,"pass schedule");
    require(near(c.interference_lower_um,4.0)&&near(c.interference_upper_um,5.0),"interference band");
    require(fin::acceptFinishedSeat(c,12.0010,20.0),"in-window shaft accepted");
    require(!fin::acceptFinishedSeat(c,12.0017,20.0),"above guarded window rejected");
    // 12.0017 measured at 26°C is 12.0017−0.000864=12.000836 at 20°C → accepted.
    require(fin::acceptFinishedSeat(c,12.0017,26.0),"temperature-corrected acceptance");

    // Bearing bore measured warm: 15.000 at 24°C → 14.99928 at 20°C.
    // j5 14.997–15.005 ∩ (15.00228…15.00528) = 15.00228–15.005 (2.72µm). With
    // U=1.0µm the guarded window is 0.72µm < 1µm and the card is refused; with
    // U=0.5µm it is 15.00278–15.0045.
    rejects([]{fin::grindCard({"SYN-SHAFT-1","S73-82_d15",15,"SYN-6002-A",15.000,24.0,"SYN-GAUGE-1",1.0});},"sub-µm window accepted");
    fin::SeatRecord w{"SYN-SHAFT-1","S73-82_d15",15,"SYN-6002-A",15.000,24.0,"SYN-GAUGE-1",0.5};
    const auto cw=fin::grindCard(w);
    require(near(cw.bearing_bore_20c_mm,15.0-0.00072),"warm bore reduced to 20°C");
    require(near(cw.guarded_lower_mm,15.00278)&&near(cw.guarded_upper_mm,15.0045),"warm-bore guarded window");

    // Gauge too coarse for a 3µm window: U=1.5µm → guarded window 0 → rejected.
    rejects([]{fin::grindCard({"S","S22.5-30.5_d12",12,"B",11.996,20,"G",1.5});},"coarse gauge accepted");
    // Bearing bore outside P6 record rejected; blank identities rejected.
    rejects([]{fin::grindCard({"S","S22.5-30.5_d12",12,"B",11.990,20,"G",0.5});},"out-of-P6 bore accepted");
    rejects([]{fin::grindCard({"","S22.5-30.5_d12",12,"B",11.996,20,"G",0.5});},"blank shaft serial accepted");
    // Bearing at nominal 12.000: j5 ∩ (12.003…12.006) = 12.003–12.005, guarded 12.0035–12.0045 with U=0.5.
    const auto cn=fin::grindCard({"S2","S22.5-30.5_d12",12,"B2",12.000,20,"G",0.5});
    require(near(cn.guarded_lower_mm,12.0035)&&near(cn.guarded_upper_mm,12.0045),"nominal-bore window");

    const auto b=fin::boreCard({"SYN-F102-1","F102",28.008,28.018,"SYN-BORE-GAUGE",2.0});
    require(near(b.guarded_lower_mm,28.010)&&near(b.guarded_upper_mm,28.016)&&near(b.target_mm,28.013),"bore guard");
    require(near(b.semi_finish_target_mm,27.863),"semi-finish undersize");
    require(fin::acceptFinishedBore(b,28.0155,20.0),"bore accepted");
    // 28.016 read at 26°C on 6082 shrinks by 28.016·0.0234·6=3.93µm → 28.01207 accepted; at 20°C it is on the edge and accepted too.
    require(fin::acceptFinishedBore(b,28.0160,26.0),"aluminium temperature correction");
    require(!fin::acceptFinishedBore(b,28.0170,20.0),"oversize bore rejected");
    rejects([]{fin::boreCard({"P","F102",28.008,28.018,"G",2.5});},"coarse bore gauge accepted");
    rejects([]{fin::boreCard({"P","F102",28.018,28.008,"G",1.0});},"inverted bore limits accepted");
    std::cout<<"finishing arithmetic regression passed (synthetic fixtures)\n";return 0;
}catch(const std::exception& e){std::cerr<<e.what()<<'\n';return 1;}
