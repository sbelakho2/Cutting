#!/usr/bin/env python3
"""Build the one canonical Cutting_Assembly_Manual.pdf (handoff section 17).

Run with a Python environment that provides reportlab (tested: reportlab 5.1.0):
    python3 tools/build_assembly_manual.py

Deterministic: the same inputs produce a semantically identical document; the
control page records source-commit and input hashes. The manual is a controlled
PRE-RELEASE build/qualification manual: nothing unverified is presented as
completed reality (handoff 14). Status labels used throughout:
    DEFINED        - identity/geometry defined in the released specs
    NOT FABRICATED - selected/designed but not built
    OPEN GATE      - verification still owed (named test/record)
"""
from __future__ import annotations

import hashlib
import pathlib

from reportlab.lib import colors
from reportlab.lib.pagesizes import A4
from reportlab.lib.styles import ParagraphStyle
from reportlab.lib.units import mm
from reportlab.platypus import (BaseDocTemplate, Flowable, Frame, Image,
                                PageBreak, PageTemplate, Paragraph, Spacer,
                                Table, TableStyle)
from reportlab.pdfgen import canvas as pdfcanvas

ROOT = pathlib.Path(__file__).resolve().parent.parent
OUT = ROOT / "release" / "Cutting_Assembly_Manual.pdf"
IMAGES = ROOT / "Cutting_Agent_Handoff_2026-10-29" / "images"
BUILD_DATE = "2026-10-29"
REVISION = "AM-PRE-1"

S = ParagraphStyle("body", fontName="Helvetica", fontSize=9, leading=12)
H1 = ParagraphStyle("h1", fontName="Helvetica-Bold", fontSize=16, leading=20, spaceAfter=8)
H2 = ParagraphStyle("h2", fontName="Helvetica-Bold", fontSize=12, leading=15, spaceBefore=8, spaceAfter=4)
H3 = ParagraphStyle("h3", fontName="Helvetica-Bold", fontSize=10, leading=13, spaceBefore=6, spaceAfter=3)
WARN = ParagraphStyle("warn", fontName="Helvetica-Bold", fontSize=9, leading=12, textColor=colors.HexColor("#8B0000"))
SMALL = ParagraphStyle("small", fontName="Helvetica", fontSize=7.5, leading=9.5)


def sha256(p: pathlib.Path) -> str:
    return hashlib.sha256(p.read_bytes()).hexdigest()[:16]


def bullets(*items: str) -> list:
    return [Paragraph(f"&bull;&nbsp; {i}", S) for i in items]


def table(rows, widths=None, header=True) -> Table:
    t = Table(rows, colWidths=widths, repeatRows=1 if header else 0)
    style = [("FONTSIZE", (0, 0), (-1, -1), 8),
             ("LEADING", (0, 0), (-1, -1), 10),
             ("GRID", (0, 0), (-1, -1), 0.4, colors.grey),
             ("VALIGN", (0, 0), (-1, -1), "TOP")]
    if header:
        style += [("BACKGROUND", (0, 0), (-1, 0), colors.HexColor("#DDE4EE")),
                  ("FONTNAME", (0, 0), (-1, 0), "Helvetica-Bold")]
    t.setStyle(TableStyle(style))
    return t


class Manual(BaseDocTemplate):
    def __init__(self, path):
        super().__init__(str(path), pagesize=A4, leftMargin=16 * mm, rightMargin=14 * mm,
                         topMargin=16 * mm, bottomMargin=14 * mm,
                         title="Cutting Assembly Manual", author="Cutting completion agent")
        frame = Frame(self.leftMargin, self.bottomMargin, self.width, self.height, id="main")
        self.addPageTemplates([PageTemplate(id="p", frames=[frame], onPage=self._decor)])

    def _decor(self, c: pdfcanvas.Canvas, doc):
        c.setFont("Helvetica", 7)
        c.setFillColor(colors.grey)
        c.drawString(16 * mm, 8 * mm, f"Cutting_Assembly_Manual.pdf  rev {REVISION}  {BUILD_DATE}  "
                                      f"PRE-RELEASE BUILD/QUALIFICATION MANUAL - not a qualification record")
        c.drawRightString(A4[0] - 14 * mm, 8 * mm, f"page {doc.page}")


class View(Flowable):
    """Vector view flowable: draws with real spec geometry on the page canvas."""

    def __init__(self, fn, height=70 * mm):
        super().__init__()
        self.fn = fn
        self.width = A4[0] - 30 * mm
        self.height = height

    def draw(self):
        self.fn(self.canv, 0, 0, self.width, self.height)


def machine_layout(c: pdfcanvas.Canvas, x, y, w, h):
    """Schematic side view with REAL station coordinates (mm from cut plane X0)."""
    c.saveState()
    c.setStrokeColor(colors.black)
    c.setLineWidth(0.7)
    base_y = y + 30
    c.line(x + 10, base_y, x + w - 10, base_y)  # wire lane
    # scale: X -800..+400 mapped to width
    def sx(X):
        return x + 10 + (X + 800.0) / 1200.0 * (w - 20)
    # cut plane
    c.setLineWidth(1.2)
    c.line(sx(0), y + 8, sx(0), y + h - 20)
    c.setFont("Helvetica", 7)
    c.drawCentredString(sx(0), y + h - 14, "cut plane X0 (blades)")
    # stations
    for X, label in [(-710, "payoff spool centre (P001)"), (-385, "wheel W0 (MM2)"),
                     (-205, "wheel W1 (MM)"), (-75.5, "left cassette nips"),
                     (75.5, "right nip (near)"), (106.75, "piece station D (candidate)"),
                     (140, "right nip (far)"), (250, "discharge sensor")]:
        c.setLineWidth(0.8)
        c.circle(sx(X), base_y, 2.2, stroke=1, fill=1)
        c.saveState()
        c.translate(sx(X), base_y - 6)
        c.rotate(60)
        c.drawString(0, 0, f"{label} [{X:g}]")
        c.restoreState()
    c.setFont("Helvetica-Bold", 7)
    c.drawCentredString(x + w / 2, y + 2, "schematic side view - real X stations, not to scale in Y (see mechanism_spec)")
    c.restoreState()


def axis_exploded(c: pdfcanvas.Canvas, x, y, w, h):
    """Exploded schematic of the S1-B cut-axis assembly (AX-01)."""
    c.saveState()
    c.setLineWidth(0.8)
    cy = y + h * 0.45
    # rail and blocks
    c.rect(x + 20, cy, w - 120, 8, stroke=1, fill=0)
    c.rect(x + 40, cy + 12, 40, 12, stroke=1, fill=0)
    c.rect(x + w - 150, cy + 12, 40, 12, stroke=1, fill=0)
    # screw
    c.line(x + 25, cy - 18, x + w - 95, cy - 18)
    c.line(x + 25, cy - 14, x + w - 95, cy - 14)
    # nut housing
    c.rect(x + w / 2 - 30, cy - 24, 60, 12, stroke=1, fill=0)
    # supports
    c.rect(x + 12, cy - 34, 18, 26, stroke=1, fill=0)
    c.rect(x + w - 92, cy - 34, 18, 26, stroke=1, fill=0)
    # motor + adapter
    c.rect(x + w - 68, cy - 30, 40, 34, stroke=1, fill=0)
    c.rect(x + w - 74, cy - 22, 6, 18, stroke=1, fill=0)
    parts = [(x + 60, cy + 24, "1 rail HGR20 400 + 2x HGH20CA (AX-R01)"),
             (x + w / 2, cy + 34, "2 carriage plate H006"),
             (x + w / 2, cy - 40, "3 ballscrew SFU1610 C7 500, 10 mm lead (AX-S01)"),
             (x + 20, cy - 46, "4 BK12/BF12 supports"),
             (x + w - 48, cy + 12, "5 motor 34E1K-85 + CL86T-V41 (DR-01)"),
             (x + w - 74, cy + 40, "6 coupling MK2/100/50/8/14 (CP-01)"),
             (x + 30, cy + 48, "7 base plate H005 (20x86x440)")]
    c.setFont("Helvetica", 7)
    for px, py, label in parts:
        c.circle(px, py, 2, stroke=1, fill=1)
        c.drawString(px + 4, py - 2, label)
    c.setFont("Helvetica-Bold", 7)
    c.drawCentredString(x + w / 2, y + 2, "exploded schematic of AX-01 (one of two axes) - ballooned to the BOM")
    c.restoreState()


def feed_cassette(c: pdfcanvas.Canvas, x, y, w, h):
    c.saveState()
    c.setLineWidth(0.8)
    c.rect(x + 30, y + h - 60, 90, 40, stroke=1, fill=0)      # housing F102
    c.circle(x + 55, y + h - 45, 12, stroke=1, fill=0)         # roller
    c.circle(x + 95, y + h - 45, 12, stroke=1, fill=0)
    c.line(x + 20, y + h - 45, x + 130, y + h - 45)            # wire
    c.rect(x + 30, y + 26, 44, 26, stroke=1, fill=0)           # motor
    c.rect(x + 140, y + h - 52, 60, 22, stroke=1, fill=0)      # encoder M202-M210
    labels = [(x + 34, y + h - 66, "housing F102/F105 + shaft F101 (x2 per cassette)"),
              (x + 34, y + h - 12, "upper nip rollers (50-75 N per nip, target 60)"),
              (x + 34, y + 20, "feed motor 24HS40-5004D-E1000 (TR-M01)"),
              (x + 142, y + h - 60, "measuring module (arm/shaft M202-M210)")]
    c.setFont("Helvetica", 7)
    for lx, ly, t in labels:
        c.drawString(lx, ly, t)
    c.setFont("Helvetica-Bold", 7)
    c.drawCentredString(x + w / 2, y + 4, "feed cassette schematic (F-series) - forces per G23")
    c.restoreState()


def payoff_view(c: pdfcanvas.Canvas, x, y, w, h):
    c.saveState()
    c.setLineWidth(0.8)
    c.circle(x + 70, y + h - 60, 34, stroke=1, fill=0)   # spool flange
    c.circle(x + 70, y + h - 60, 8, stroke=1, fill=0)    # hub
    c.line(x + 30, y + h - 60, x + w - 40, y + h - 60)   # spindle
    c.rect(x + 130, y + h - 74, 10, 28, stroke=1, fill=0)  # post
    c.rect(x + w - 70, y + h - 74, 10, 28, stroke=1, fill=0)
    c.rect(x + 30, y + 30, w - 60, 10, stroke=1, fill=0)   # base P003
    c.rect(x + w - 60, y + h - 52, 22, 12, stroke=1, fill=0)  # brake disc/caliper
    labels = [(x + 40, y + h - 16, "P001 spool 400x3 flanges + 60x2.5 hub"),
              (x + 140, y + h - 84, "P004/P005 posts + SKF 6204-2Z/C3 (PY-B01)"),
              (x + 40, y + 20, "P003 base 430x560x8, 4xM12 anchors"),
              (x + w - 58, y + h - 62, "P006/P007 friction brake (NOT a safety brake)")]
    c.setFont("Helvetica", 7)
    for lx, ly, t in labels:
        c.drawString(lx, ly, t)
    c.setFont("Helvetica-Bold", 7)
    c.drawCentredString(x + w / 2, y + 4, "payoff schematic (docs/26 R2) - <=30 kg spool; keyed boss P008 torque path")
    c.restoreState()


def build() -> None:
    OUT.parent.mkdir(parents=True, exist_ok=True)
    doc = Manual(OUT)
    st = []
    st += [Paragraph("Cutting S1-B — Visual Assembly Manual", H1),
           Paragraph(f"Revision {REVISION} &nbsp;|&nbsp; build {BUILD_DATE} &nbsp;|&nbsp; "
                     "PRE-RELEASE BUILD/QUALIFICATION MANUAL", WARN),
           Paragraph("This is the single canonical assembly manual for the S1-B wire "
                     "cutting/strip machine. It is a controlled pre-release document: the design "
                     "is defined in the released specifications, but physical fabrication and "
                     "qualification are NOT complete. Nothing in this manual is presented as a "
                     "completed qualification record. Every section carries its status; open "
                     "gates are listed in section 10 and must close on their named physical tests "
                     "before the machine is used for production.", S), Spacer(1, 4)]

    st += [Paragraph("1. Document control", H2),
           table([["field", "value"],
                  ["Document", "release/Cutting_Assembly_Manual.pdf (the one canonical PDF)"],
                  ["Source commit", "branch completion/2026-10-29 (pinned base 0f28aa3)"],
                  ["BOM authority", f"bom/*.csv (sha256/16: selected-cutting-head {sha256(ROOT / 'bom/selected-cutting-head.csv')})"],
                  ["Spec authority", f"shared/*.hpp (sha256/16: controls_spec {sha256(ROOT / 'shared/controls_spec.hpp')})"],
                  ["Signal contract", f"requirements/signal-table.md (sha256/16: {sha256(ROOT / 'requirements/signal-table.md')})"],
                  ["Software", "software/machine (host tests green incl. observability/waste/negative-control groups)"],
                  ["Status vocabulary", "DEFINED / NOT FABRICATED / OPEN GATE (see handoff 14)"],
                  ["Not included", "no qualification results, no delivered-cost claim, no safety certification"]],
                 widths=[28 * mm, 150 * mm])]

    st += [Paragraph("2. Safety warnings (read before any construction work)", H2)]
    st += [Paragraph("WARNING — the items below are construction and operating hazards. "
                     "They are not a substitute for the risk assessment and safety validation "
                     "(P03/P04/P12) that remain open.", WARN)]
    st += bullets(
        "Mains and the 36/60 V DC buses carry lethal energy. Lock out and verify de-energised before opening the cabinet.",
        "The cutting blades are hardened, sharp and spring/cam driven. Never reach into the head; the E-stop removes energy but stored energy must be released (M21/M37 records).",
        "The payoff spool weighs up to 30 kg (plus wire). Two-person lift or a lifting aid; the spool retention and brake are NOT safety brakes and provide no guaranteed stop (docs/26).",
        "The blade/axis proof loads (2500 N axial class) are structural loads for the machine, never body loads. No person may occupy the load path during any powered test.",
        "Waste stubs/slugs are ejected into a sweep chute. The chute sensor is a handling channel, not a safety device.",
        "Electrical diagrams in this manual are functional references; the electrical release (branch protection, conductor sizing, terminal torque) is an OPEN GATE (G32).",
    )

    st += [PageBreak(), Paragraph("3. Machine overview and station map", H2),
           Paragraph("Coordinate contract: X = wire travel, Y = opposing cutting strokes, "
                     "Z = up; cut plane X0; wire at Y0/Z112; +X toward the piece/eject side. "
                     "All dimensions in mm. The view below shows every real X station used by "
                     "the sensing/handling contract.", S),
           View(machine_layout, height=64 * mm),
           Paragraph("Stations (real coordinates): payoff spool centre X=-710; measuring wheels "
                     "W0/W1 at X=-385/-205 (stock side - they can never observe a severed piece); "
                     "left cassette nips at X=-75.5/-140; right cassette nips at X=75.5/140 "
                     "(a severed body shorter than 140 mm cannot be propelled by them); candidate "
                     "piece station D at X=106.75; discharge clearance sensor zone X=230..270; "
                     "waste chute sensor is on the fall path below the blade pocket.", S)]

    st += [Paragraph("4. Subassembly work structure", H2),
           Paragraph("Build order: (A) base and frame; (B) two cutting axes; (C) head and "
                     "blade tooling; (D) feed cassettes and measuring modules; (E) payoff; "
                     "(F) guards; (G) cabinet and wiring; (H) commissioning. Each section below "
                     "gives identities, fasteners, tools, inspection points and warnings. "
                     "Torque values shown as PENDING are open gates: do not guess them.", S)]

    st += [Paragraph("4.1 (A) Base and frame — H001/H002 + welded frame", H3),
           table([["item", "identity", "qty", "fasteners / torque", "inspection", "status"],
                  ["base datum", "H001 ground datum 180x1040x20 steel", 1,
                   "M12 floor anchors PENDING (site)", "flatness to drawing PENDING", "DEFINED / NOT FABRICATED"],
                  ["risers", "H002 riser 110x355x20 steel", 2,
                   "per drawing PENDING", "stage grid 46/100 verify", "DEFINED / NOT FABRICATED"],
                  ["frame", "welded frame (mechanism frame group FR_)", 1,
                   "weld procedure PENDING", "weld inspection PENDING", "DEFINED / NOT FABRICATED"]],
                 widths=[22 * mm, 52 * mm, 10 * mm, 40 * mm, 32 * mm, 28 * mm])]

    st += [Paragraph("4.2 (B) Cutting axes — AX-01 x2 (see exploded schematic)", H3),
           table([["balloon", "identity", "qty/axis", "fasteners / torque", "inspection", "status"],
                  ["1", "AX-R01 HGR20 400 rail + 2x HGH20CA", 2,
                   "M5 rail bolts at 60 pitch; torque PENDING", "rail straightness; block preload", "DEFINED / NOT FABRICATED"],
                  ["2", "H006 carriage plate 60x75 pad at 46", 1,
                   "4x HGH20CA lands; ISO4762 M5x16 cl.10.9 (HF0-B) + ISO7092 washer; torque PENDING",
                   "49.97 F0 register; PCD70 M5", "DEFINED / NOT FABRICATED"],
                  ["3", "AX-S01 SFU1610 C7 500, 10 mm lead + DSG16h", 1,
                   "nut flange screws PENDING", "backlash/runout PENDING; end machining 8 h7x18", "DEFINED / NOT FABRICATED"],
                  ["4", "BK12/BF12 supports", "1 set",
                   "M5 lands; torque PENDING", "support alignment PENDING", "DEFINED / NOT FABRICATED"],
                  ["5", "DR-01 34E1K-85 + CL86T-V41", 1,
                   "HM-B ISO4762 M5x25 cl.10.9 + HM-W ISO7092 washer; 15 mm thread engagement nominal; torque PENDING",
                   "air gap; 134.1 mm rear envelope clearance", "DEFINED / NOT FABRICATED"],
                  ["6", "CP-01 R+W MK2/100/50/8/14 (10 Nm)", 1,
                   "hub clamping screws per R+W instructions PENDING", "8H7/14H7 bores; runout", "DEFINED / NOT FABRICATED"],
                  ["7", "H005 base plate 20x86x440", 1,
                   "6x 6.6 base grid 46/100; torque PENDING", "rail seats milled; BK/BF lands", "DEFINED / NOT FABRICATED"]],
                 widths=[12 * mm, 44 * mm, 14 * mm, 52 * mm, 38 * mm, 24 * mm])]
    st += [Paragraph("WARNING — proof loading (2500 N axial / 750 N transverse / 375 N vertical, "
                     "double) is a controlled validation procedure (P05), not an assembly step. "
                     "The 4900 N class support rating is below the 5000 N proof load: this is an "
                     "OPEN GATE (G37) and must never be closed by rounding.", WARN)]

    st += [Paragraph("4.3 (C) Head and blade tooling — H004/T001/H007", H3),
           table([["item", "identity", "qty", "fasteners / torque", "inspection", "status"],
                  ["holder", "H004 monolithic holder 42CrMo4+QT 650 MPa min yield", 2,
                   "per drawing PENDING", "ground blade seat; load path per STEP", "DEFINED / NOT FABRICATED"],
                  ["blades", "T001 matched 90 deg V pair, 49x38x3", "1 pair",
                   "shimmed seat; clamp torque PENDING", "edge radius; loaded face gap 0.020 mm (G22)", "DEFINED / NOT FABRICATED"],
                  ["backing", "H007 backing plate 30x46x8", 2,
                   "torque PENDING", "shim record; 1200 N blade reaction path", "DEFINED / NOT FABRICATED"]],
                 widths=[20 * mm, 52 * mm, 12 * mm, 42 * mm, 38 * mm, 26 * mm])]

    st += [PageBreak(), Paragraph("4.4 (D) Feed cassettes and measuring modules", H3),
           Paragraph("One left (stock) and one right (piece) cassette; two measuring wheel "
                     "modules W0/W1 on the stock side (X=-385/-205). The downstream piece "
                     "station D (X=106.75 candidate) is an OPEN GATE (M02 fit study).", S)]

    st += [Paragraph("4.5 (E) Payoff (docs/26 R2)", H3),
           Paragraph("Assemble P003 base -> P004/P005 posts with SKF 6204-2Z/C3 bearings -> "
                     "P002 spindle with M20x1.5 retention -> P008 keyed drive boss welded to the "
                     "left flange INSIDE the hub tube -> P001 spool -> P006/P007 brake. The brake "
                     "is effective only with P008 fitted. The spool centre sits at X=-710.", S)]

    st += [Paragraph("5. Wiring and signals", H2),
           Paragraph("Functional reference only — the electrical release is an OPEN GATE. "
                     "The authoritative signal contract is requirements/signal-table.md; summary:", S),
           table([["signal", "meaning", "status"],
                  ["GP0-7", "step/dir to 4 drivers via TPL7407LDR sinks", "DEFINED"],
                  ["GP8-11", "W0/W1 quadrature via AM26LV32E (ifm RU3100, TTL 2048 PPR)", "DEFINED"],
                  ["GP12-16,18-21,27", "panel/safety/ALM/home coupler inputs (LOW = active)", "DEFINED"],
                  ["GP17,22,26,28", "drive enable, LEDs, buzzer", "DEFINED"],
                  ["piece station D", "quadrature of the downstream module", "OPEN GATE (Phase H pin)"],
                  ["part_clear", "discharge present->clear evidence; fails closed while unassigned", "OPEN GATE (Phase H pin)"],
                  ["waste_clear / waste_sweep", "waste-chute transit + sweep actuator; fails closed", "OPEN GATE (Phase H pin)"]],
                 widths=[36 * mm, 100 * mm, 42 * mm])]

    st += [Paragraph("6. Commissioning and qualification flow", H2)]
    st += bullets(
        "Flash the firmware image, record its hash, then run the factory-test UF2 (separate image) to exercise every I/O channel.",
        "Enter service fixture mode only for engineering runs (never emits accepted counts or approval).",
        "Home both blades; route wire through the cassettes; run the commissioning polarity moves; calibrate all three measuring stations (W0/W1/D).",
        "Approve the retained recipe over the host link only after commissioning; production START requires this approval.",
        "Execute the physical qualification: P00-P15 families plus the 18 TS and 17 TF thermal tests (tests/P10-thermal/procedures.md).",
        "Close every open gate in section 10 with signed records; only then may this manual's status change.",
    )

    st += [Paragraph("7. Maintenance (provisional — P14/TS evidence owed)", H2),
           table([["item", "interval", "limit / action", "status"],
                  ["filters (two-path ventilation)", "inspect each shift; replace per TS evidence", "flow < service limit triggers change", "OPEN GATE"],
                  ["blade edges T001", "inspect per shift", "edge radius/wear limit from coupons (G03)", "OPEN GATE"],
                  ["measuring wheels W0/W1/D", "weekly", "tread wear/calibration drift check (G24)", "OPEN GATE"],
                  ["nips and guides", "weekly", "50-75 N nip force check (G23)", "OPEN GATE"],
                  ["payoff brake pads", "monthly", "friction pad wear (docs/26)", "OPEN GATE"],
                  ["bearing lubrication", "per P14 duty test", "interval to be established", "OPEN GATE"]],
                 widths=[52 * mm, 46 * mm, 52 * mm, 28 * mm])]

    st += [Paragraph("8. Fault and recovery (operator actions)", H2),
           table([["fault", "meaning", "operator action"],
                  ["red LED steady", "latched fault; drives disabled", "clear cause; hold RESET 0.5 s; rehome"],
                  ["no_wire / length_slip", "material motion did not match the plan", "inspect wire path and wheels; rethread; RESET"],
                  ["part_not_cleared", "discharge evidence missing (quarantine)", "remove/verify the part in the takeaway; log the scrap"],
                  ["waste_not_cleared", "stub/slug did not transit the chute", "clear the sweep chute and pocket; RESET"],
                  ["wire_geometry", "planned move/score rejected by the wire model", "do not force; rethread and rehome; report"],
                  ["power loss mid-piece", "in-flight part has uncertain state", "on restart: rehome + rethread; the prime pass scrap-discharges leftovers"]],
                 widths=[38 * mm, 60 * mm, 80 * mm])]

    st += [PageBreak(), Paragraph("9. Spare parts (initial list)", H2),
           table([["part", "identity", "qty", "status"],
                  ["blades", "T001 matched 90 deg V pair", "2 pairs", "DEFINED / NOT FABRICATED"],
                  ["filters", "Pfannenberg 11666102055 / 11760002055 candidates", "1 set", "OPEN GATE (TS evidence)"],
                  ["measuring wheels", "ifm E60137", "1", "DEFINED"],
                  ["encoders", "ifm RU3100", "1", "DEFINED"],
                  ["fuses/relays", "per electrical release", "-", "OPEN GATE (G32)"]],
                 widths=[30 * mm, 90 * mm, 20 * mm, 38 * mm])]

    st += [Paragraph("10. Open-gate register (must close before production)", H2),
           table([["gate", "what is owed", "record"],
                  ["G37 axis loads", "revalidate acceptance loads vs the frozen 10 mm-lead screw; resolve 4900 N < 5000 N proof", "P05"],
                  ["G16 fits", "3-6 um interference matched pairs + bearing clearance envelope", "P01/P13"],
                  ["G18 shaft thermal", "shaft rise <= 20 K above housing (stored 29.66 K case fails)", "P10"],
                  ["G22 face gap", "0.020 mm loaded gap interval from cut-quality data", "P07/P13"],
                  ["G29-G30 ventilation", "installed filtered flow and thermal margins; 18 TS + 17 TF", "P10 + tests/P10-thermal"],
                  ["G32 protection", "exact fuse/breaker coordination and wire/terminal limits", "P02/P04"],
                  ["G35 safety", "risk-derived PLr/SIL, dual-channel validation by a competent safety engineer", "P03/P04/P12"],
                  ["Phase H I/O", "assign part_clear / waste_clear / waste_sweep / piece-station pins", "signal-table"],
                  ["G39 RFQs", "send the drafted enquiries (couplings, axis, protection, custom work, Tunisia delivery)", "procurement records"],
                  ["G47 cost", "complete landed cost + 70 % like-for-like comparison", "cost rollup"]],
                 widths=[34 * mm, 110 * mm, 34 * mm])]

    st += [Paragraph("11. Photographic reference", H2),
           Paragraph("Layout-inspiration photos from the machine under construction. They are "
                     "reference images only: they establish no ratings, dimensions or "
                     "qualification (handoff 3.1/12).", S)]
    for img, cap in [("machine-front.jpg", "front view (reference)"),
                     ("machine-rear.jpg", "rear view (reference)"),
                     ("payoff-and-buffer.jpg", "payoff and buffer (reference)")]:
        p = IMAGES / img
        if p.exists():
            st += [Image(str(p), width=70 * mm, height=52 * mm), Paragraph(cap, SMALL), Spacer(1, 3)]

    st += [Paragraph("12. Vector views", H2),
           Paragraph("Schematic vector views generated from the released spec geometry with real "
                     "station coordinates and ballooned BOM identities. They are assembly aids, "
                     "not native-CAD renders; the native CAD views remain part of the release "
                     "gate and must not be substituted by these schematics.", S),
           View(axis_exploded, height=70 * mm), PageBreak(),
           View(feed_cassette, height=64 * mm), Spacer(1, 4),
           View(payoff_view, height=64 * mm)]
    doc.build(st)
    print(f"wrote {OUT.relative_to(ROOT)} ({OUT.stat().st_size} bytes)")


if __name__ == "__main__":
    build()
