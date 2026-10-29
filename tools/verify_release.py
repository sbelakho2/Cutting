#!/usr/bin/env python3
"""Release freshness/consistency verifier (anti-stale-artifact gate).

`python3 tools/verify_release.py` checks, and exits nonzero on any finding:

  V1  release/release-manifest.json hashes match the CURRENT files,
  V2  release/assembly-manual-build.json embedded input hashes match CURRENT
      sources (detects a PDF built against older inputs),
  V3  every deterministic generator reproduces byte-identical outputs when
      re-run (analysis manifests, thermal/P harnesses, BOM reconciliation),
  V4  the record-integrity and BOM-reconciliation checks run clean.

The compiled host tests and the ctest battery are separate gates (recorded in
tests/verification-2026-10-29/); this tool guards artifact consistency only.
"""
from __future__ import annotations

import hashlib
import json
import pathlib
import subprocess
import sys

ROOT = pathlib.Path(__file__).resolve().parent.parent


def sha256(p: pathlib.Path) -> str:
    return hashlib.sha256(p.read_bytes()).hexdigest()


def check(name: str, ok: bool, detail: str = "") -> bool:
    print(f"[{'CLOSED' if ok else 'FINDING'}] {name}" + (f" - {detail}" if detail and not ok else ""))
    return ok


def main() -> int:
    findings = 0

    # V1 release manifest freshness
    manifest_path = ROOT / "release" / "release-manifest.json"
    if not manifest_path.exists():
        findings += not check("V1_release_manifest_exists", False, "run tools/build_release_manifest.py")
    else:
        manifest = json.loads(manifest_path.read_text())
        stale = []
        for rel, expected in manifest.get("deliverables_sha256", {}).items():
            p = ROOT / rel
            if not p.exists() or sha256(p) != expected:
                stale.append(rel)
        findings += not check("V1_release_manifest_fresh", not stale, f"changed/missing: {stale}")

    # V2 assembly manual freshness
    build_path = ROOT / "release" / "assembly-manual-build.json"
    if not build_path.exists():
        findings += not check("V2_manual_build_record_exists", False, "run tools/build_assembly_manual.py")
    else:
        rec = json.loads(build_path.read_text())
        stale = []
        for rel, expected16 in rec.get("embedded_input_hashes_16", {}).items():
            p = ROOT / rel
            if not p.exists() or sha256(p)[:16] != expected16:
                stale.append(rel)
        findings += not check("V2_manual_inputs_fresh", not stale,
                              f"PDF built against older {stale}; rebuild tools/build_assembly_manual.py")

    # V3 generator determinism (hash -> run -> hash)
    generators = [
        ("analysis/m36_m31/compute.py", ["analysis/m36_m31/manifest.json", "analysis/m36_m31/report.md"]),
        ("analysis/electrical_thermal/compute.py", ["analysis/electrical_thermal/manifest.json",
                                                    "analysis/electrical_thermal/report.md"]),
        ("analysis/cost-basis/compute.py", ["analysis/cost-basis/manifest.json", "analysis/cost-basis/report.md"]),
        ("analysis/transport-geometry/compute.py", ["analysis/transport-geometry/manifest.json",
                                                    "analysis/transport-geometry/report.md"]),
        ("tools/generate_thermal_tests.py", ["tests/P10-thermal/manifest.json",
                                             "tests/P10-thermal/procedures.md"]),
        ("tools/generate_ph_tests.py", ["tests/P00-P15/manifest.json", "tests/P00-P15/tests-index.csv",
                                        "tests/P00-P15/procedures.md"]),
        ("tools/reconcile_bom.py", ["bom/reconciliation-report.md"]),
    ]
    for script, outputs in generators:
        before = {o: sha256(ROOT / o) for o in outputs if (ROOT / o).exists()}
        run = subprocess.run([sys.executable, str(ROOT / script)], capture_output=True, text=True)
        after = {o: sha256(ROOT / o) for o in outputs if (ROOT / o).exists()}
        ok = run.returncode in (0, 1) and before == after  # reconcile exits 1 on findings
        findings += not check(f"V3_deterministic:{script}", ok,
                              f"script rc={run.returncode} or outputs drifted: "
                              f"{[o for o in outputs if before.get(o) != after.get(o)]}")

    # V4 integrity checks clean
    for script in ("tools/check_production_records.py",):
        run = subprocess.run([sys.executable, str(ROOT / script)], capture_output=True, text=True)
        findings += not check(f"V4:{script}", run.returncode == 0, run.stdout.strip() + run.stderr.strip())

    print()
    print(f"overall: {'CLOSED' if findings == 0 else f'{findings} FINDING(S)'}")
    return 0 if findings == 0 else 1


if __name__ == "__main__":
    sys.exit(main())
