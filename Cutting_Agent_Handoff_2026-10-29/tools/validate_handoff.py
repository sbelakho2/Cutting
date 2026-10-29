#!/usr/bin/env python3
"""Validate handoff coverage/packaging, not machine adequacy or qualification."""
import argparse
import csv
import hashlib
import json
import pathlib
import re


def rows(path):
    with path.open(newline="", encoding="utf-8-sig") as stream:
        return list(csv.DictReader(stream))


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("root", type=pathlib.Path, nargs="?", default=pathlib.Path(__file__).resolve().parents[1])
    parser.add_argument("--write-checksums", action="store_true")
    args = parser.parse_args()
    root = args.root.resolve()
    doc = (root / "ENGINEERING_HANDOFF.md").read_text()
    models = re.findall(r"^### (M\d{2}) — (.+)$", doc, re.M)
    assert [item[0] for item in models] == [f"M{i:02}" for i in range(1, 49)], "48 ordered model packages required"
    assert re.findall(r"^## (\d+)\.", doc, re.M) == [str(i) for i in range(21)], "Sections 0-20 must be present"
    assert len(rows(root / "data/original-47-goals.csv")) == 47
    assert len(rows(root / "data/historical-parts-2026-10-13.csv")) == 217
    assert len(rows(root / "data/historical-fabrication-2026-10-13.csv")) == 54
    assert len(rows(root / "data/historical-prices-2026-10-13.csv")) == 52
    goals = rows(root / "data/goal-verification-map.csv")
    assert [int(g["goal_id"]) for g in goals] == list(range(1, 48))
    model_set = {m[0] for m in models}
    for goal in goals:
        assert set(goal["model_ids"].split(";")) <= model_set
        assert goal["physical_test_ids"] and goal["acceptance_source"]
    registry = rows(root / "data/model-workpackages.csv")
    assert [(r["model_id"], r["title"]) for r in registry] == models
    assert all(r["execution_state"] == "work_instruction_not_executed_by_handoff" for r in registry)
    assert len(rows(root / "data/thermal-steady-plan.csv")) == 18
    assert len(rows(root / "data/thermal-fault-plan.csv")) == 17
    prices = rows(root / "data/fresh-price-observations-2026-10-29.csv")
    assert len(prices) == 5 and all(p["observed_date"] == "2026-10-29" for p in prices)
    tree = rows(root / "evidence/repository-tree.csv")
    assert len(tree) == 1374, f"Unexpected pinned tree count: {len(tree)}"
    assert not any(p["path"].startswith(("docs/45", "docs/46")) for p in tree)
    tree_by_path = {p["path"]: p for p in tree}
    for fact in rows(root / "evidence/pinned-source-facts.csv"):
        assert tree_by_path[fact["repo_path"]]["object_sha"] == fact["blob_sha"]
    provenance = json.loads((root / "evidence/review-provenance.json").read_text())
    assert len(provenance["host_test_groups"]) == 21
    assert not provenance["physical_test_performed"]
    counterexample = json.loads((root / "evidence/stock-side-counterexample/result.json").read_text())
    assert counterexample["counterexample_reproduced"] and not counterexample["physical_test"]
    assert len(list((root / "images").glob("*.jpg"))) == 3
    assert (root / "history/Cutting_Design_Audit_2026-10-13.md").is_file()
    assert (root / "history/engineering-audit-2026-10-13.txt").is_file()
    required = [
        "START_HERE.md", "tools/run_host_checks.sh", "tools/reproduce_stock_side_mismatch.py",
        "data/part-record-template.json", "data/evidence-record-template.json",
    ]
    for name in required:
        assert (root / name).is_file(), name
    for path in root.rglob("*"):
        if not path.is_file():
            continue
        assert path.suffix.lower() != ".pdf", "The handoff must contain no PDF"
        assert path.read_bytes()[:4] != b"\x7fELF", f"Exclude compiled binary {path}"
        assert not path.name.endswith((".pyc", ".zip")), f"Unexpected generated/cache file {path}"
    files = sorted(p for p in root.rglob("*") if p.is_file() and p.name != "SHA256SUMS.txt")
    checksums = "".join(f"{hashlib.sha256(p.read_bytes()).hexdigest()}  {p.relative_to(root).as_posix()}\n" for p in files)
    if args.write_checksums:
        (root / "SHA256SUMS.txt").write_text(checksums)
    else:
        assert (root / "SHA256SUMS.txt").read_text() == checksums, "Checksum mismatch"
    print(json.dumps({
        "handoff_packaging_valid": True,
        "machine_qualified": False,
        "original_goals": len(goals),
        "model_work_instructions": len(models),
        "thermal_steady_plans": 18,
        "thermal_fault_plans": 17,
        "pinned_repo_paths": len(tree),
        "fresh_limited_price_observations": len(prices),
        "package_files_excluding_checksum": len(files),
        "pdf_files_in_handoff": 0,
    }, indent=2))


if __name__ == "__main__":
    main()
