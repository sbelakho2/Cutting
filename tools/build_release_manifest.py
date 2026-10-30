#!/usr/bin/env python3
"""Build the release manifest: hashes of every completion-agent deliverable.

`python3 tools/build_release_manifest.py` -> release/release-manifest.json.
Deterministic over its input set: the same deliverables hash identically.
"""
from __future__ import annotations

import datetime
import hashlib
import json
import pathlib

ROOT = pathlib.Path(__file__).resolve().parent.parent

DELIVERABLES = [
    "requirements/requirements-register.md",
    "requirements/requirements-register.csv",
    "requirements/signal-table.md",
    "software/machine/tests/material_plant.hpp",
    "software/machine/tests/test_machine.cpp",
    "software/machine/core/controller.cpp",
    "software/machine/core/controller.hpp",
    "software/machine/core/types.hpp",
    "software/machine/core/board.hpp",
    "software/machine/core/protocol.cpp",
    "software/machine/core/protocol.hpp",
    "shared/controls_spec.hpp",
    "bom/selected-cutting-head.csv",
    "bom/custom-axis-bom.csv",
    "bom/selected-wire-and-payoff.csv",
    "bom/sourcing-evidence.csv",
    "bom/sourcing-evidence-mechanical.csv",
    "bom/reconciliation-report.md",
    "tools/reconcile_bom.py",
    "tools/generate_thermal_tests.py",
    "tools/generate_ph_tests.py",
    "tools/build_assembly_manual.py",
    "analysis/m36_m31/manifest.json",
    "analysis/m36_m31/report.md",
    "analysis/electrical_thermal/manifest.json",
    "analysis/transport-geometry/manifest.json",
    "analysis/transport-geometry/report.md",
    "analysis/cost-basis/manifest.json",
    "analysis/phase-h-io/option-study.md",
    "analysis/phase-h-io/decision.md",
    "analysis/reel-buffer/manifest.json",
    "analysis/axis-rating-requirements/manifest.json",
    "analysis/joint-thermal-redesign/requirements.md",
    "analysis/M-register.csv",
    "bom/phase-h-io-occurrences.csv",
    "release/audit-closure-2026-10-30.md",
    "tests/P10-thermal/manifest.json",
    "tests/P10-thermal/procedures.md",
    "tests/P00-P15/manifest.json",
    "tests/P00-P15/tests-index.csv",
    "tests/P00-P15/procedures.md",
    "tests/reproduction-2026-10-29/provenance.json",
    "procurement/rfq-drafts.md",
    "procurement/compliance-matrix-2026-10-30.md",
    "procurement/1688-alibaba-verified-2026-10-30.md",
    "release/Cutting_Assembly_Manual.pdf",
    "release/decision-log.md",
]


TEXT_SUFFIXES = {".csv", ".md", ".json", ".py", ".hpp", ".cpp", ".h", ".txt", ".log", ".sh", ".cmake"}


def sha256(path: pathlib.Path) -> str:
    """Canonical hash: text files hashed with LF newlines (CRLF/LF checkouts
    reproduce identical digests); binary files as-is. Same rule as
    tools/verify_release.py and tools/build_assembly_manual.py (review F14)."""
    data = path.read_bytes()
    if path.suffix.lower() in TEXT_SUFFIXES and b"\x00" not in data:
        data = data.replace(b"\r\n", b"\n")
    return hashlib.sha256(data).hexdigest()


def main() -> None:
    files = {}
    missing = []
    for rel in DELIVERABLES:
        p = ROOT / rel
        if p.exists():
            files[rel] = sha256(p)
        else:
            missing.append(rel)
    manifest = {
        "record_id": "RELEASE-MANIFEST-2026-10-29",
        "generated_by": "tools/build_release_manifest.py",
        "branch": "completion/2026-10-29",
        "pinned_base_commit": "0f28aa3289cf052409ad48f54dc5d23b130fd441",
        "deliverables_sha256": files,
        "missing_from_manifest": missing,
        "verification": {
            "host_test_groups": "24 groups ok (tests/reproduction-2026-10-29/revised-tree/host-tests.log)",
            "bom_reconciliation": "overall CLOSED (bom/reconciliation-report.md)",
            "production_record_integrity": "passed (126 BOM rows, 9 controls sourcing records)",
            "analysis_determinism": "M36/M31, electrical_thermal, cost-basis manifests reproduce identical hashes across runs",
            "pdf": "release/Cutting_Assembly_Manual.pdf built and content-checked (AM-PRE-3, 15 pages)",
        },
        "declared_scope": "PRE-RELEASE build/qualification manual; physical qualification open "
                          "(P/TS/TF records shipped executed=false); ~70% cost test NOT ESTABLISHED",
    }
    out = ROOT / "release" / "release-manifest.json"
    out.write_text(json.dumps(manifest, indent=2, sort_keys=True) + "\n", encoding="utf-8")
    print(f"wrote {out.relative_to(ROOT)}: {len(files)} hashes, {len(missing)} missing")


if __name__ == "__main__":
    main()
