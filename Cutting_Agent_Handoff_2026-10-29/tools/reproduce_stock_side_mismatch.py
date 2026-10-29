#!/usr/bin/env python3
"""Counterexample: both measuring wheels follow stock-side wire, not severed part.
Usage: python3 reproduce_stock_side_mismatch.py /path/to/Cutting --out evidence
This changes only a copied host fixture. The repository controller is unmodified.
It is a kinematic counterexample, not a simulation of cutting/contact forces.
"""
import argparse, pathlib, subprocess, json, hashlib
p=argparse.ArgumentParser();p.add_argument('repo',type=pathlib.Path);p.add_argument('--out',type=pathlib.Path,default=pathlib.Path('stock-side-check'));a=p.parse_args()
r=a.repo.resolve();o=a.out.resolve();o.mkdir(parents=True,exist_ok=True)
src=r/'software/machine/tests/sim_board.hpp';s=src.read_text()
old='''            if (!wire_present || encoder_freeze[side]) return;
            enc_accum_[side] += d * encoder_sign[side] * (1.0 + slip_ppm[side] * 1e-6) * ctrl::enc::nominal_counts_per_metre / 1e6;
            const double whole = std::trunc(enc_accum_[side]);
            encoder_[side] += static_cast<std::int64_t>(whole);
            enc_accum_[side] -= whole;'''
new='''            // Both actual wheel stations X=-205/-385 are on continuous stock.
            // In the synchronized uncut feed the stock displacement follows L.
            // A severed piece moved only by R cannot move either stock wheel.
            if (side != 0 || !wire_present) return;
            for (int wheel=0;wheel<2;++wheel) {
                if (encoder_freeze[wheel]) continue;
                enc_accum_[wheel] += d * encoder_sign[wheel] * (1.0+slip_ppm[wheel]*1e-6) * ctrl::enc::nominal_counts_per_metre / 1e6;
                const double whole=std::trunc(enc_accum_[wheel]);
                encoder_[wheel]+=static_cast<std::int64_t>(whole);
                enc_accum_[wheel]-=whole;
            }'''
if s.count(old)!=1: raise SystemExit('Fixture layout changed: review adaptation; do not silently patch a different model')
(o/'stock_side_board.hpp').write_text(s.replace(old,new))
main=r'''#include "controller.hpp"
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
'''
(o/'main.cpp').write_text(main)
cmd=['g++','-std=c++20','-O1','-Wall','-Wextra','-Werror','-fsanitize=address,undefined','-fno-omit-frame-pointer','-I'+str(r/'shared'),'-I'+str(r/'software/machine/core'),'-I'+str(o),str(r/'software/machine/core/controller.cpp'),str(r/'software/machine/core/profile.cpp'),str(r/'software/machine/core/protocol.cpp'),str(o/'main.cpp'),'-o',str(o/'counterexample')]
subprocess.run(cmd,check=True)
import os
env=os.environ.copy();env['ASAN_OPTIONS']='detect_leaks=0'
run=subprocess.run([str(o/'counterexample')],capture_output=True,text=True,env=env)
(o/'result.log').write_text(run.stdout+run.stderr)
metadata={'repo_commit':subprocess.check_output(['git','-C',str(r),'rev-parse','HEAD'],text=True).strip(),'controller_sha256':hashlib.sha256((r/'software/machine/core/controller.cpp').read_bytes()).hexdigest(),'original_fixture_sha256':hashlib.sha256(src.read_bytes()).hexdigest(),'adapted_fixture_sha256':hashlib.sha256((o/'stock_side_board.hpp').read_bytes()).hexdigest(),'physical_test':False,'scope':'stock-side sensor kinematics; same upstream controller','counterexample_reproduced':run.returncode==0,'returncode':run.returncode}
(o/'result.json').write_text(json.dumps(metadata,indent=2)+'\n')
print(run.stdout,run.stderr,end='')
raise SystemExit(run.returncode)
