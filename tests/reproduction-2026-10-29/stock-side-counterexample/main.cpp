#include "controller.hpp"
#include "stock_side_board.hpp"
#include <cstdio>
using namespace starz; using namespace starz::fw;
int main(){
 sim::SimBoard b; Controller c(b); c.init();
 auto advance=[&](unsigned ms){for(unsigned i=0;i<ms;++i){c.tick();b.advance(ctrl::mcu::control_period_us);}};
 advance(ctrl::timing::input_debounce_ms+5);
 b.field.start=true;advance(ctrl::timing::start_hold_ms+ctrl::timing::input_debounce_ms+10);b.field.start=false;
 for(unsigned i=0;i<ctrl::timing::homing_timeout_ms+1000 && c.state()!=State::Ready && c.state()!=State::Fault;++i)advance(1);
 if(c.state()!=State::Ready){std::puts("setup_failed");return 3;}
 Job j{};j.length_um=300000;j.wire_diameter_um=2500;j.quantity=1;
 if(c.load_job(j)!=CommandResult::Accepted || c.approve()!=CommandResult::Accepted || c.start()!=CommandResult::Accepted){std::puts("setup_failed");return 4;}
 for(unsigned i=0;i<60000 && c.state()!=State::Fault && c.state()!=State::Ready;++i)advance(1);
 const auto t=c.telemetry();
 std::printf("state=%u faults=0x%08x pieces_done=%u pieces_accepted=%u left_um=%.3f right_um=%.3f enc0=%lld enc1=%lld\n",unsigned(c.state()),unsigned(c.faults()),unsigned(t.counters.pieces_done),unsigned(t.counters.pieces_accepted),b.feed_true_um[0],b.feed_true_um[1],(long long)b.encoder_[0],(long long)b.encoder_[1]);
 const bool reproduced=c.state()==State::Fault && (c.faults()&fault::length_slip) && t.counters.pieces_accepted==0;
 std::puts(reproduced?"COUNTEREXAMPLE_REPRODUCED: stock wheels cannot verify severed-piece ejection":"COUNTEREXAMPLE_NOT_REPRODUCED: inspect source or fixture");
 return reproduced?0:5;
}
