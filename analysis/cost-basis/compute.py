#!/usr/bin/env python3
"""G47/U04 cost basis: dated observations vs the historical baseline register.

`python3 analysis/cost-basis/compute.py` writes report.md + manifest.json.
What this tool computes is an EVIDENCE-LEDGER of observed merchandise prices
with their currencies and dates, and an explicit NOT-COSTED set. It does not
claim a build cost and does not claim the ~70 % target: freight, duty, labour,
tooling, metrology, qualification and consumables are not yet costed (blank is
not zero), and the like-for-like baseline mapping (handoff 12.2, decision D6)
remains an owner/completion gate. No price is invented and no currency is
converted without a dated conversion observation.
"""
from __future__ import annotations

import csv
import hashlib
import json
import pathlib

ROOT = pathlib.Path(__file__).resolve().parent.parent.parent
HERE = pathlib.Path(__file__).resolve().parent
BOM = ROOT / "bom"
DATA = ROOT / "Cutting_Agent_Handoff_2026-10-29" / "data"


def sha256(p: pathlib.Path) -> str:
    return hashlib.sha256(p.read_bytes()).hexdigest()[:16]


def load(path: pathlib.Path):
    with path.open(newline="", encoding="utf-8-sig") as f:
        return list(csv.DictReader(f))


def main() -> None:
    hist = load(DATA / "historical-prices-2026-10-13.csv")
    ev = load(BOM / "sourcing-evidence.csv")

    hist_totals: dict[str, float] = {}
    hist_count = 0
    for r in hist:
        try:
            price = float(r["Numeric unit price"])
        except ValueError:
            continue
        cur = r["Currency"]
        hist_totals[cur] = hist_totals.get(cur, 0.0) + price
        hist_count += 1

    obs_totals: dict[str, float] = {}
    obs_rows = []
    not_costed = 0
    for r in ev:
        price = (r.get("observed_unit_price") or "").strip()
        cur = (r.get("currency") or "").strip()
        if price and cur:
            try:
                obs_totals[cur] = obs_totals.get(cur, 0.0) + float(price)
            except ValueError:
                continue
            obs_rows.append({"item": r["item_id"], "price": price, "currency": cur,
                             "date": r.get("observation_date", ""), "basis": r.get("price_basis", "")})
        else:
            not_costed += 1

    result = {
        "model": "G47/U04 cost evidence ledger (not a build-cost claim)",
        "inputs": {
            "historical-prices-2026-10-13.csv": sha256(DATA / "historical-prices-2026-10-13.csv"),
            "bom/sourcing-evidence.csv": sha256(BOM / "sourcing-evidence.csv"),
        },
        "historical_baseline_observations": {
            "count": hist_count,
            "sums_by_currency": hist_totals,
            "note": "dated 2026-10-13 public observations; a sum of observations is not a build cost",
        },
        "current_observations": {
            "count": len(obs_rows),
            "sums_by_currency": obs_totals,
            "rows": obs_rows,
            "note": "dated 2026-10-13/2026-10-29 merchandise observations; no freight/duty; no conversion",
        },
        "not_costed_evidence_rows": not_costed,
        "open_cost_elements": [
            "freight and duty to Tunisia (Incoterm/site = owner decision D1)",
            "custom machining/finishing quotes (D5 dual route)",
            "labour and assembly time",
            "inspection and metrology",
            "qualification testing",
            "consumables (blades, filters, lubrication)",
        ],
        "seventy_percent_test": {
            "status": "NOT ESTABLISHED",
            "reason": "neither side of C_new <= 0.30 * C_baseline is complete and like-for-like "
                      "scope mapping (12.2) is an open decision; reporting a saving now would "
                      "fabricate it",
        },
    }
    (HERE / "manifest.json").write_text(json.dumps(result, indent=2, sort_keys=True) + "\n", encoding="utf-8")

    lines = ["# Cost evidence ledger (G47/U04) - not a build-cost claim", "",
             "Deterministic output of `analysis/cost-basis/compute.py`.", "",
             f"- Historical baseline register: {hist_count} dated observations (2026-10-13), sums by currency: "
             + ", ".join(f"{k} {v:.2f}" for k, v in sorted(hist_totals.items())),
             f"- Current dated observations: {len(obs_rows)} rows, sums by currency: "
             + ", ".join(f"{k} {v:.2f}" for k, v in sorted(obs_totals.items())),
             f"- Evidence rows without cost: {not_costed} (blank is not zero - each is an open quote)",
             "- Open cost elements: freight/duty (D1), machining quotes (D5), labour, metrology, "
             "qualification, consumables.",
             "- ~70 % target test: NOT ESTABLISHED - both sides incomplete and like-for-like scope",
             "  mapping (12.2) open (D6). Any saving figure at this stage would be fabricated.", "",
             "Currency sums are arithmetic sums of single observations at actual purchase",
             "quantities; they are ledger facts, not basket prices. No currency conversion is",
             "performed without a dated conversion observation."]
    (HERE / "report.md").write_text("\n".join(lines) + "\n", encoding="utf-8")
    print(f"historical={hist_count} rows, current={len(obs_rows)} rows, not_costed={not_costed}")
    print("seventy_percent_test: NOT ESTABLISHED (both sides incomplete)")


if __name__ == "__main__":
    main()
