#!/usr/bin/env python3
"""Build the one canonical Cutting_Assembly_Manual.pdf (handoff section 17).

Run with a Python environment providing reportlab (tested: reportlab 5.1.0):
    python3 tools/build_assembly_manual.py

Deterministic over its inputs; release/assembly-manual-build.json records the
source hashes embedded in the document-control page, and tools/verify_release.py
detects a stale build. The manual is a controlled PRE-RELEASE build/
qualification manual: nothing unverified is presented as completed reality
(handoff 14). Status labels used throughout:
    DEFINED        - identity/geometry defined in the released specs/BOM
    NOT FABRICATED - selected/designed but not yet built
    OPEN GATE      - verification or release action still owed (named record)
"""
from __future__ import annotations

import hashlib
import json
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
REVISION = "AM-PRE-2"

INK = colors.HexColor("#1a1a1a")
ACCENT = colors.HexColor("#204a80")
WARN_RED = colors.HexColor("#8B0000")
GREY = colors.HexColor("#555555")
LIGHT = colors.HexColor("#EDF1F7")

S = ParagraphStyle("body", fontName="Helvetica", fontSize=8.5, leading=11.2, textColor=INK)
SB = ParagraphStyle("bodyb", parent=S, fontName="Helvetica-Bold")
H1 = ParagraphStyle("h1", fontName="Helvetica-Bold", fontSize=17, leading=21, textColor=ACCENT, spaceAfter=8)
H2 = ParagraphStyle("h2", fontName="Helvetica-Bold", fontSize=12, leading=15, textColor=ACCENT,
                    spaceBefore=9, spaceAfter=4)
H3 = ParagraphStyle("h3", fontName="Helvetica-Bold", fontSize=9.5, leading=12, spaceBefore=6, spaceAfter=2)
WARN = ParagraphStyle("warn", parent=SB, textColor=WARN_RED)
CAP = ParagraphStyle("cap", parent=S, fontSize=7.5, leading=9.5, textColor=GREY)
TOC = ParagraphStyle("toc", parent=S, fontSize=9, leading=13)


def sha256(p: pathlib.Path) -> str:
    return hashlib.sha256(p.read_bytes()).hexdigest()


def bullets(*items) -> list:
    return [Paragraph(f"&bull;&nbsp;&nbsp;{i}", S) for i in items]


def numbered(*items) -> list:
    return [Paragraph(f"{n}.&nbsp;&nbsp;{i}", S) for n, i in enumerate(items, 1)]


def table(rows, widths=None, header=True, fs=7.5) -> Table:
    cell = ParagraphStyle("cell", parent=S, fontSize=fs, leading=fs + 2.2)
    head = ParagraphStyle("head", parent=cell, fontName="Helvetica-Bold")
    wrapped = []
    for i, row in enumerate(rows):
        wrapped.append([v if isinstance(v, (Table, Image)) else
                        Paragraph(str(v), head if (header and i == 0) else cell)
                        for v in row])
    t = Table(wrapped, colWidths=widths, repeatRows=1 if header else 0)
    style = [("GRID", (0, 0), (-1, -1), 0.35, colors.HexColor("#8A94A6")),
             ("VALIGN", (0, 0), (-1, -1), "TOP"),
             ("LEFTPADDING", (0, 0), (-1, -1), 3),
             ("RIGHTPADDING", (0, 0), (-1, -1), 3)]
    if header:
        style += [("BACKGROUND", (0, 0), (-1, 0), LIGHT)]
    t.setStyle(TableStyle(style))
    return t


# ----------------------------------------------------------------- drawing kit
def centerline(c, x1, y1, x2, y2):
    c.saveState()
    c.setStrokeColor(colors.HexColor("#7a7a7a"))
    c.setDash([6, 2, 1, 2], 0)
    c.setLineWidth(0.35)
    c.line(x1, y1, x2, y2)
    c.restoreState()


def dim_h(c, x1, x2, y, label, tick=1.4):
    c.saveState()
    c.setStrokeColor(INK)
    c.setLineWidth(0.5)
    c.line(x1, y, x2, y)
    for x in (x1, x2):
        c.line(x, y - tick, x, y + tick)
    c.setFont("Helvetica", 6.5)
    c.setFillColor(INK)
    c.drawCentredString((x1 + x2) / 2, y + 1.6, label)
    c.restoreState()


def dim_v(c, y1, y2, x, label, tick=1.4):
    c.saveState()
    c.setStrokeColor(INK)
    c.setLineWidth(0.5)
    c.line(x, y1, x, y2)
    for y in (y1, y2):
        c.line(x - tick, y, x + tick, y)
    c.setFont("Helvetica", 6.5)
    c.setFillColor(INK)
    c.saveState()
    c.translate(x - 1.8, (y1 + y2) / 2)
    c.rotate(90)
    c.drawCentredString(0, 0, label)
    c.restoreState()
    c.restoreState()


def balloon(c, x, y, n, tx=None, ty=None, r=4.6):
    c.saveState()
    if tx is not None:
        c.setStrokeColor(ACCENT)
        c.setLineWidth(0.5)
        c.line(x, y, tx, ty)
    c.setFillColor(colors.white)
    c.setStrokeColor(ACCENT)
    c.setLineWidth(0.7)
    c.circle(x, y, r, stroke=1, fill=1)
    c.setFillColor(ACCENT)
    c.setFont("Helvetica-Bold", 7)
    c.drawCentredString(x, y - 2.4, str(n))
    c.restoreState()


def hatch(c, x, y, w, h, step=2.2):
    c.saveState()
    c.setStrokeColor(colors.HexColor("#9aa3b0"))
    c.setLineWidth(0.3)
    p = c.beginPath()
    p.rect(x, y, w, h)
    c.clipPath(p, stroke=0, fill=0)
    t = x - h
    while t < x + w + h:
        c.line(t, y, t + h, y + h)
        t += step
    c.restoreState()


def view_title(c, x, y, w, title, sub=None):
    c.saveState()
    c.setFont("Helvetica-Bold", 8)
    c.setFillColor(ACCENT)
    c.drawCentredString(x + w / 2, y, title)
    if sub:
        c.setFont("Helvetica", 6.8)
        c.setFillColor(GREY)
        c.drawCentredString(x + w / 2, y - 8, sub)
    c.restoreState()


class View(Flowable):
    """Vector sheet flowable: a drawing function receives (canvas, 0, 0, w, h)."""

    def __init__(self, fn, height=72 * mm):
        super().__init__()
        self.fn = fn
        self.width = A4[0] - 32 * mm
        self.height = height

    def draw(self):
        self.fn(self.canv, 0, 0, self.width, self.height)


# --------------------------------------------------------------- vector sheets
def sheet_machine_elevation(c, x, y, w, h):
    """Full machine side elevation with the real station dimension chain."""
    base_y = y + 42
    def sx(X):
        return x + 26 + (X + 800.0) / 1200.0 * (w - 52)
    c.saveState()
    # frame and wire lane
    c.setStrokeColor(INK)
    c.setLineWidth(1.1)
    c.rect(x + 20, base_y - 24, w - 40, 14, stroke=1, fill=0)      # frame body
    c.setLineWidth(0.8)
    c.line(x + 26, base_y, x + w - 26, base_y)                      # wire lane
    centerline(c, x + 26, base_y, x + w - 26, base_y)
    # payoff
    c.setLineWidth(0.9)
    c.circle(sx(-710), base_y + 6, 17, stroke=1, fill=0)
    c.circle(sx(-710), base_y + 6, 4.5, stroke=1, fill=0)
    c.rect(sx(-710) - 3, base_y - 24, 6, 24, stroke=1, fill=0)
    # stock wheels
    for X in (-385.0, -205.0):
        c.circle(sx(X), base_y + 7, 5.2, stroke=1, fill=0)
        c.line(sx(X), base_y, sx(X), base_y + 2)
    # cassettes
    for X0, X1 in ((-140.0, -75.5), (75.5, 140.0)):
        c.rect(sx(X0) - 8, base_y + 1, sx(X1) - sx(X0) + 16, 11, stroke=1, fill=0)
        c.circle(sx(X0), base_y + 6.5, 4, stroke=1, fill=0)
        c.circle(sx(X1), base_y + 6.5, 4, stroke=1, fill=0)
    # head at cut plane
    c.setLineWidth(1.2)
    c.rect(sx(-30), base_y + 12, 60, 18, stroke=1, fill=0)
    c.setLineWidth(1.4)
    c.line(sx(0), base_y + 2, sx(0), base_y + 42)
    # discharge zone + takeaway
    c.setStrokeColor(WARN_RED)
    c.setLineWidth(0.9)
    c.setDash([3, 2], 0)
    c.line(sx(230), base_y - 6, sx(230), base_y + 14)
    c.line(sx(270), base_y - 6, sx(270), base_y + 14)
    c.setDash()
    c.setStrokeColor(INK)
    c.rect(sx(280), base_y - 10, 70, 10, stroke=1, fill=0)
    # cabinet
    c.rect(x + w - 62, base_y - 22, 34, 52, stroke=1, fill=0)
    # station marks + leaders
    stations = [(-710, "payoff spool centre", "P001/PY-W01"),
                (-385, "measuring wheel W0", "MM2 / RU3100"),
                (-205, "measuring wheel W1", "MM / RU3100"),
                (-140, "left nip (far)", "F-series"),
                (-75.5, "left nip (near)", "F-series"),
                (0, "cut plane X0", "head + T001 blades"),
                (75.5, "right nip (near)", "F-series"),
                (106.75, "piece station D (cand.)", "M02 fit OPEN GATE"),
                (140, "right nip (far)", "F-series"),
                (250, "discharge sensor zone", "X230-270, Phase-H pin")]
    c.setFont("Helvetica", 5.6)
    for i, (X, name, ref) in enumerate(stations):
        px = sx(X)
        c.setFillColor(INK)
        c.circle(px, base_y, 1.7, stroke=1, fill=1)
        ly = base_y + 52 + (i % 2) * 11
        c.setStrokeColor(colors.HexColor("#9098a4"))
        c.setLineWidth(0.35)
        c.line(px, base_y + 2, px, ly - 2)
        c.setFillColor(INK)
        c.drawCentredString(px, ly, f"{name}  [{X:g}]")
        c.setFillColor(GREY)
        c.drawCentredString(px, ly - 6, ref)
    # dimension chain along the bottom
    dim_h(c, sx(-710), sx(0), y + 12, "710")
    dim_h(c, sx(0), sx(250), y + 12, "250")
    dim_v(c, y + 20, base_y + 30, x + 14, "Y see CAD")
    view_title(c, x, y + h - 12, w,
               "MACHINE SIDE ELEVATION - real X stations (mm from cut plane X0)",
               "schematic assembly view generated from released spec geometry - not a native-CAD render")
    c.restoreState()


def sheet_axis_exploded(c, x, y, w, h):
    c.saveState()
    cy = y + h * 0.42
    # rail + blocks
    c.setStrokeColor(INK)
    c.setLineWidth(0.9)
    c.rect(x + 34, cy, w - 170, 9, stroke=1, fill=0)
    hatch(c, x + 34, cy, w - 170, 9, 2.6)
    c.rect(x + 62, cy + 13, 52, 13, stroke=1, fill=0)
    c.rect(x + w - 226, cy + 13, 52, 13, stroke=1, fill=0)
    # screw + journals
    c.setLineWidth(0.7)
    c.line(x + 40, cy - 22, x + w - 176, cy - 22)
    c.line(x + 40, cy - 17, x + w - 176, cy - 17)
    c.rect(x + 30, cy - 24, 12, 10, stroke=1, fill=0)      # BK12
    c.rect(x + w - 180, cy - 24, 12, 10, stroke=1, fill=0)  # BF12
    # nut housing on carriage
    c.rect(x + w / 2 - 38, cy - 30, 76, 13, stroke=1, fill=0)
    # base plate under everything
    c.setLineWidth(0.8)
    c.rect(x + 22, cy - 52, w - 120, 10, stroke=1, fill=0)
    hatch(c, x + 22, cy - 52, w - 120, 10, 3.0)
    # motor + adapter + coupling
    c.rect(x + w - 108, cy - 34, 56, 40, stroke=1, fill=0)   # motor 134.1 body
    c.rect(x + w - 122, cy - 24, 14, 20, stroke=1, fill=0)   # adapter 15 mm
    c.rect(x + w - 142, cy - 20, 20, 12, stroke=1, fill=0)   # coupling
    centerline(c, x + 30, cy - 19.5, x + w - 52, cy - 19.5)
    # balloons
    items = [(x + 88, cy + 34, 1, "1 rail + blocks AX-R01 (2 sets/axis)"),
             (x + w / 2, cy + 42, 2, "2 carriage plate H006"),
             (x + w / 2, cy - 44, 3, "3 ballscrew AX-S01 (SFU1610 C7, 10 mm lead)"),
             (x + 36, cy - 36, 4, "4 BK12/BF12 supports"),
             (x + w - 52, cy + 16, 5, "5 motor kit DR-01 (34E1K-85 + CL86T-V41)"),
             (x + w - 132, cy + 12, 6, "6 coupling CP-01 (MK2/100/50/8/14)"),
             (x + 52, cy - 62, 7, "7 base plate H005"),
             (x + w - 84, cy - 44, 8, "8 adapter H003 (15 mm R5)")]
    c.setFont("Helvetica", 6.4)
    for bx, by, n, label in items:
        anchor_x = bx if n in (2, 3, 7) else bx
        balloon(c, bx, by, n, anchor_x, by - 10 if by > cy else by + 10)
        c.setFillColor(INK)
        tx = bx + 7 if bx < x + w / 2 else bx + 7
        c.drawString(tx, by - 2, label)
    dim_h(c, x + 34, x + w - 136, y + 26, "rail 440 overall / 400 between blocks")
    dim_h(c, x + w - 122, x + w - 52, y + 26, "134.1 body + 21.8 encoder + 37 shaft")
    dim_v(c, cy - 52, cy + 26, x + 12, "axis envelope per docs/39")
    view_title(c, x, y + h - 12, w,
               "AX-01 CUT-AXIS ASSEMBLY - exploded schematic (one of two axes)",
               "balloons map to bom/custom-axis-bom.csv; loads per docs/39 - acceptance-load revalidation OPEN GATE (G37/P05)")
    c.restoreState()


def sheet_head_assembly(c, x, y, w, h):
    c.saveState()
    cx, cy = x + w * 0.38, y + h * 0.42
    # holder (front view) with backbone and nose pocket
    c.setStrokeColor(INK)
    c.setLineWidth(1.0)
    c.rect(cx - 70, cy - 10, 140, 34, stroke=1, fill=0)
    hatch(c, cx - 70, cy - 10, 140, 34, 3.2)
    c.setFillColor(colors.white)
    c.rect(cx - 16, cy + 2, 32, 12, stroke=1, fill=1)   # blade seat pocket
    c.setFillColor(INK)
    c.rect(cx - 12, cy + 4, 24, 8, stroke=1, fill=0)    # blade pair
    # blade detail circle
    c.setStrokeColor(ACCENT)
    c.setLineWidth(0.6)
    c.circle(x + w - 78, cy + 40, 26, stroke=1, fill=0)
    c.line(cx + 8, cy + 10, x + w - 100, cy + 32)
    c.setLineWidth(0.8)
    c.line(x + w - 92, cy + 46, x + w - 64, cy + 34)     # V blade 1
    c.line(x + w - 92, cy + 34, x + w - 64, cy + 46)     # V blade 2
    c.setFont("Helvetica", 5.8)
    c.setFillColor(ACCENT)
    c.drawCentredString(x + w - 78, cy + 62, "90 deg V pair T001 49x38x3")
    c.drawCentredString(x + w - 78, cy + 12, "loaded face gap 0.020 nom")
    # backing + shims behind
    c.setFillColor(INK)
    c.rect(cx - 20, cy - 2, 40, 4, stroke=1, fill=0)
    # screw axis of the stage behind
    centerline(c, cx - 90, cy - 20, cx + 90, cy - 20)
    c.rect(cx - 30, cy - 26, 60, 8, stroke=1, fill=0)
    items = [(cx - 70, cy + 34, 1, "H004 holder 42CrMo4+QT (650 MPa min yield)"),
             (cx, cy + 30, 2, "T001 matched blade pair"),
             (cx - 20, cy - 12, 3, "H007 backing 30x46x8 + shims"),
             (cx + 30, cy - 32, 4, "stage interface (carriage H006)")]
    c.setFont("Helvetica", 6.4)
    for bx, by, n, label in items:
        balloon(c, bx, by, n, bx, by - 9)
        c.setFillColor(INK)
        c.drawString(bx + 7, by - 2, label)
    dim_h(c, cx - 16, cx + 16, y + 30, "blade seat pocket (nose only, R3)")
    view_title(c, x, y + h - 12, w,
               "CUTTING HEAD - assembly front view + blade detail",
               "gap procedure: see work instruction C-4; shim record required (G22)")
    c.restoreState()


def sheet_feed_cassette(c, x, y, w, h):
    c.saveState()
    hx, hy = x + 60, y + h * 0.52
    # housing with shaft and rollers
    c.setStrokeColor(INK)
    c.setLineWidth(0.9)
    c.rect(hx, hy - 26, 110, 44, stroke=1, fill=0)
    c.circle(hx + 28, hy - 4, 12, stroke=1, fill=0)
    c.circle(hx + 82, hy - 4, 12, stroke=1, fill=0)
    c.circle(hx + 28, hy - 4, 3, stroke=1, fill=0)
    c.circle(hx + 82, hy - 4, 3, stroke=1, fill=0)
    centerline(c, hx + 10, hy - 4, hx + 100, hy - 4)
    c.line(hx - 12, hy - 4, hx + 122, hy - 4)              # wire
    # upper nip rollers + springs
    c.rect(hx + 18, hy + 14, 20, 12, stroke=1, fill=0)
    c.rect(hx + 72, hy + 14, 20, 12, stroke=1, fill=0)
    # motor behind
    c.rect(hx + 10, hy - 58, 52, 26, stroke=1, fill=0)
    # encoder module on shaft end
    c.rect(hx + 128, hy - 16, 34, 24, stroke=1, fill=0)
    items = [(hx + 24, hy + 36, 1, "upper nip rollers (50-75 N each, target 60)"),
             (hx + 55, hy + 46, 2, "housing F102/F105 + shaft F101 (x2)"),
             (hx + 36, hy - 66, 3, "feed motor TR-M01 (24HS40-5004D-E1000)"),
             (hx + 145, hy + 16, 4, "measuring module M202-M210 + RU3100"),
             (hx + 55, hy - 34, 5, "lower driven rollers (UMCL40 class)")]
    c.setFont("Helvetica", 6.4)
    for bx, by, n, label in items:
        balloon(c, bx, by, n, bx, by - 9 if by > hy else by + 9)
        c.setFillColor(INK)
        c.drawString(bx + 7, by - 2, label)
    # belt side view mini
    c.setStrokeColor(INK)
    c.setLineWidth(0.7)
    c.circle(x + w - 90, hy + 8, 17, stroke=1, fill=0)
    c.circle(x + w - 90, hy + 8, 5, stroke=1, fill=0)
    c.circle(x + w - 48, hy - 18, 11, stroke=1, fill=0)
    c.line(x + w - 90, hy + 25, x + w - 48, hy - 7)
    c.line(x + w - 90, hy - 9, x + w - 48, hy - 29)
    c.setFont("Helvetica", 5.8)
    c.setFillColor(GREY)
    c.drawCentredString(x + w - 69, hy + 34, "HTD5M-225-25 belt / 20T pulley")
    c.drawCentredString(x + w - 69, hy - 40, "tooth-pass 79.5775 Hz @0.50 m/s")
    dim_h(c, hx + 16, hx + 94, y + 24, "roller centres 54 (46/100 variants per cassette)")
    view_title(c, x, y + h - 12, w,
               "FEED CASSETTE - schematic (one of two: stock side + piece side)",
               "nip forces G23, wheel forces G24 measured per P06; belt tension 180-200 N both stations")
    c.restoreState()


def sheet_payoff(c, x, y, w, h):
    c.saveState()
    px, py = x + 96, y + h * 0.52
    c.setStrokeColor(INK)
    c.setLineWidth(0.9)
    # spool
    c.circle(px, py, 36, stroke=1, fill=0)
    c.circle(px, py, 34, stroke=1, fill=0)
    c.circle(px, py, 8, stroke=1, fill=0)
    for r in (12, 20, 28):
        c.circle(px, py, r, stroke=1, fill=0)
    # spindle + posts + base
    c.setLineWidth(1.0)
    c.line(px - 44, py, x + w - 120, py)
    c.rect(x + w - 196, py - 26, 12, 52, stroke=1, fill=0)
    c.rect(x + w - 104, py - 26, 12, 52, stroke=1, fill=0)
    hatch(c, x + w - 196, py - 26, 12, 52, 2.6)
    hatch(c, x + w - 104, py - 26, 12, 52, 2.6)
    c.rect(x + w - 216, y + 34, 152, 12, stroke=1, fill=0)
    hatch(c, x + w - 216, y + 34, 152, 12, 3.0)
    # brake disc + caliper
    c.rect(x + w - 84, py - 12, 6, 24, stroke=1, fill=0)
    c.rect(x + w - 78, py - 7, 14, 14, stroke=1, fill=0)
    # bearings
    c.rect(x + w - 200, py - 8, 8, 16, stroke=1, fill=0)
    c.rect(x + w - 108, py - 8, 8, 16, stroke=1, fill=0)
    items = [(px, py + 46, 1, "P001 spool: 2x S235JR 400x3 flanges + 60x2.5 ERW hub"),
             (x + w - 150, py + 30, 2, "P002 spindle C45 20 h6 L660 + M20x1.5 retention"),
             (x + w - 190, py + 6, 3, "P004/P005 posts + PY-B01 SKF 6204-2Z/C3"),
             (x + w - 140, y + 28, 4, "P003 base 430x560x8 (4x M12 anchors)"),
             (x + w - 60, py + 16, 5, "P006/P007 friction brake (NOT a safety brake)"),
             (px - 30, py - 42, 6, "P008 keyed drive boss (6 JS9) - torque path")]
    c.setFont("Helvetica", 6.4)
    for bx, by, n, label in items:
        balloon(c, bx, by, n, bx, by - 9)
        c.setFillColor(INK)
        c.drawString(bx + 7, by - 2, label)
    dim_h(c, px, x + w - 196, y + 18, "spool centre X=-710 (station map)")
    dim_v(c, py - 36, py + 36, px - 44, "spool flange 400")
    view_title(c, x, y + h - 12, w,
               "PAYOFF - exploded schematic (docs/26 R2)",
               "<=30 kg spool + wire; two-person lift; keyed boss P008 required for brake torque path")
    c.restoreState()


def sheet_cabinet_layout(c, x, y, w, h):
    c.saveState()
    ox, oy = x + 40, y + 40
    cw, ch = w - 80, h - 70
    c.setStrokeColor(INK)
    c.setLineWidth(1.0)
    c.rect(ox, oy, cw, ch, stroke=1, fill=0)
    c.setFont("Helvetica", 6.2)
    comps = [
        (0.04, 0.72, 0.20, 0.22, "XPSUAF13AP safety relay (CT-SR01)", "dual-channel E-stop/guard"),
        (0.27, 0.72, 0.20, 0.22, "CP-E24V/10A 24 V PSU (CT-PS01)", "logic/sensor supply"),
        (0.50, 0.72, 0.22, 0.22, "LE-350-60 x2 (PS-01)", "60 V axes, NOT paralleled"),
        (0.04, 0.40, 0.16, 0.26, "CL86T-V41 (DR-01)", "head driver"),
        (0.23, 0.40, 0.16, 0.26, "CL57T-V41 x2 (TR-D01)", "feed drivers"),
        (0.42, 0.40, 0.16, 0.26, "LC1D12BD + coils", "K1-K3 contactors"),
        (0.61, 0.40, 0.16, 0.26, "Pico 2 (CT-MCU01)", "controller + carrier PCB"),
        (0.04, 0.06, 0.30, 0.26, "terminal rows + PLC-RSC couplers", "field I/O isolation"),
        (0.38, 0.06, 0.20, 0.26, "bleed/absorption (G33 OPEN)", "per-branch, measured decay owed"),
        (0.61, 0.06, 0.34, 0.26, "filter fan PF66000 x2 (TS-gated)", "two-path ventilation"),
    ]
    for fx, fy, fw, fh, name, note in comps:
        bx, by = ox + fx * cw, oy + fy * ch
        bwi, bhi = fw * cw, fh * ch
        c.setLineWidth(0.6)
        c.rect(bx, by, bwi, bhi, stroke=1, fill=0)
        c.setFillColor(INK)
        c.drawString(bx + 3, by + bhi - 8, name)
        c.setFillColor(GREY)
        c.drawString(bx + 3, by + bhi - 15, note)
    c.setFont("Helvetica", 6.5)
    c.setFillColor(WARN_RED)
    c.drawString(ox + 4, oy + ch + 6, "layout schematic - terminal/wire schedule follows the electrical release (G32 OPEN GATE)")
    view_title(c, x, y + h - 12, w,
               "CABINET LAYOUT (Rittal AE 1180.500 class) - schematic arrangement",
               "energisation prohibited until the electrical release closes G32/G33")
    c.restoreState()


def sheet_wiring_blocks(c, x, y, w, h):
    c.saveState()
    blocks = [
        (0.06, 0.68, "230 V AC in", "L/N/PE -> main switch + SPD"),
        (0.30, 0.68, "protection", "fuses/breakers per branch (G32)"),
        (0.54, 0.68, "24 V PSU", "logic, couplers, sensors"),
        (0.78, 0.68, "60 V PSUs", "LE-350-60 x2 (isolated buses)"),
        (0.06, 0.40, "RP2350 core", "Pico 2 + carrier PCB"),
        (0.30, 0.40, "drivers", "CL86T + 2x CL57T, ENA common"),
        (0.54, 0.40, "encoders", "W0/W1 (GP8-11) + D (Phase-H)"),
        (0.78, 0.40, "handling I/O", "part_clear/waste_* (Phase-H)"),
        (0.06, 0.12, "safety chain", "E-stop + guard -> XPSUAF13AP -> ENA"),
        (0.30, 0.12, "panel", "START/STOP/RESET + lamps + buzzer"),
        (0.54, 0.12, "ALM loop", "4x driver ALM -> GP18-21"),
        (0.78, 0.12, "USB-CDC", "optional host link only"),
    ]
    for fx, fy, name, note in blocks:
        bx, by = x + fx * w, y + fy * h
        c.setStrokeColor(ACCENT if "safety" in name or "handling" in name else INK)
        c.setLineWidth(0.8)
        c.rect(bx, by, w * 0.18, h * 0.16, stroke=1, fill=0)
        c.setFillColor(INK)
        c.setFont("Helvetica-Bold", 6.2)
        c.drawCentredString(bx + w * 0.09, by + h * 0.11, name)
        c.setFont("Helvetica", 5.4)
        c.setFillColor(GREY)
        c.drawCentredString(bx + w * 0.09, by + h * 0.045, note)
    arrows = [((0.24, 0.74), (0.30, 0.74)), ((0.48, 0.74), (0.54, 0.74)), ((0.72, 0.74), (0.78, 0.74)),
              ((0.15, 0.68), (0.15, 0.56)), ((0.39, 0.68), (0.39, 0.56)), ((0.63, 0.68), (0.63, 0.56)),
              ((0.87, 0.68), (0.87, 0.56)), ((0.15, 0.40), (0.15, 0.28)), ((0.39, 0.40), (0.39, 0.28)),
              ((0.63, 0.40), (0.63, 0.28)), ((0.87, 0.40), (0.87, 0.28))]
    c.setStrokeColor(colors.HexColor("#66717f"))
    c.setLineWidth(0.7)
    for (x1, y1), (x2, y2) in arrows:
        c.line(x + x1 * w, y + y1 * h, x + x2 * w, y + y2 * h)
    view_title(c, x, y + h - 12, w,
               "FUNCTIONAL WIRING BLOCKS - reference only (electrical release G32 OPEN)",
               "signals and polarity: requirements/signal-table.md; every handling channel fails closed while unassigned")
    c.restoreState()


def sheet_handling_path(c, x, y, w, h):
    c.saveState()
    lane = y + h * 0.55
    c.setStrokeColor(INK)
    c.setLineWidth(0.8)
    c.line(x + 20, lane, x + w - 20, lane)
    centerline(c, x + 20, lane, x + w - 20, lane)
    # blade pocket + chute + bin
    c.rect(x + w * 0.30, lane - 22, 44, 16, stroke=1, fill=0)
    c.setLineWidth(0.7)
    c.line(x + w * 0.32, lane - 22, x + w * 0.36, lane - 52)
    c.line(x + w * 0.38, lane - 22, x + w * 0.42, lane - 52)
    c.rect(x + w * 0.33, lane - 66, 34, 14, stroke=1, fill=0)
    c.setFont("Helvetica", 5.8)
    c.setFillColor(INK)
    c.drawCentredString(x + w * 0.36, lane - 76, "waste bin")
    c.drawCentredString(x + w * 0.36, lane - 12, "blade pocket + sweep (waste_sweep)")
    c.drawCentredString(x + w * 0.36, lane - 42, "chute sensor (waste_clear)")
    # takeaway
    c.rect(x + w * 0.56, lane - 14, 120, 12, stroke=1, fill=0)
    c.drawCentredString(x + w * 0.62, lane - 24, "takeaway: >=130 mm slide past grip loss (M30)")
    c.setStrokeColor(WARN_RED)
    c.setDash([3, 2], 0)
    c.line(x + w * 0.50, lane - 8, x + w * 0.50, lane + 16)
    c.line(x + w * 0.545, lane - 8, x + w * 0.545, lane + 16)
    c.setDash()
    c.setFillColor(WARN_RED)
    c.drawCentredString(x + w * 0.522, lane + 20, "discharge zone X230-270 (part_clear)")
    c.setFillColor(INK)
    # flow arrows for the three body classes
    flows = [("10 mm prime stub", lane + 34, "sweep -> chute"),
             ("3-25 mm slugs", lane + 44, "release at open -> sweep -> chute"),
             ("200-3000 mm part", lane + 54, "grip to X140 -> coast -> takeaway -> clear")]
    for label, ly, note in flows:
        c.setFont("Helvetica", 6.4)
        c.drawString(x + 24, ly, label)
        c.setFillColor(GREY)
        c.drawString(x + 120, ly, note)
        c.setFillColor(INK)
    view_title(c, x, y + h - 12, w,
               "HANDLING AND WASTE PATHS - body classes and required evidence",
               "acceptance needs discharge present->clear in window; waste needs chute transit; bodies <140 mm are never propel-lable")
    c.restoreState()


# --------------------------------------------------------------------- manual
class Manual(BaseDocTemplate):
    def __init__(self, path):
        super().__init__(str(path), pagesize=A4, leftMargin=16 * mm, rightMargin=14 * mm,
                         topMargin=15 * mm, bottomMargin=14 * mm,
                         title="Cutting S1-B Assembly Manual", author="Cutting completion agent",
                         subject="Controlled pre-release build/qualification manual")
        frame = Frame(self.leftMargin, self.bottomMargin, self.width, self.height, id="main")
        self.addPageTemplates([PageTemplate(id="p", frames=[frame], onPage=self._decor)])

    def _decor(self, c, doc):
        c.setStrokeColor(colors.HexColor("#C3CBD8"))
        c.setLineWidth(0.5)
        c.line(16 * mm, A4[1] - 11 * mm, A4[0] - 14 * mm, A4[1] - 11 * mm)
        c.setFont("Helvetica", 6.6)
        c.setFillColor(GREY)
        c.drawString(16 * mm, A4[1] - 9.4 * mm,
                     "CUTTING S1-B ASSEMBLY MANUAL  |  rev " + REVISION + "  |  " + BUILD_DATE)
        c.drawRightString(A4[0] - 14 * mm, A4[1] - 9.4 * mm,
                          "PRE-RELEASE BUILD/QUALIFICATION MANUAL - not a qualification record")
        c.line(16 * mm, 10 * mm, A4[0] - 14 * mm, 10 * mm)
        c.drawString(16 * mm, 6.6 * mm, "release/Cutting_Assembly_Manual.pdf (single canonical PDF)")
        c.drawRightString(A4[0] - 14 * mm, 6.6 * mm, f"page {doc.page}")


def build() -> None:
    OUT.parent.mkdir(parents=True, exist_ok=True)
    doc = Manual(OUT)
    st = []
    W = A4[0] - 32 * mm

    # ---------------------------------------------------------------- cover
    st += [Spacer(1, 22 * mm),
           Paragraph("CUTTING S1-B", ParagraphStyle("t0", parent=H1, fontSize=30, leading=34,
                                                    alignment=1, textColor=ACCENT)),
           Paragraph("VISUAL ASSEMBLY MANUAL",
                     ParagraphStyle("t1", parent=H1, fontSize=16, alignment=1, textColor=INK)),
           Spacer(1, 6 * mm),
           Paragraph(f"Revision {REVISION} &nbsp;&bull;&nbsp; {BUILD_DATE} &nbsp;&bull;&nbsp; "
                     "controlled document",
                     ParagraphStyle("t2", parent=S, fontSize=10, alignment=1)),
           Spacer(1, 4 * mm),
           Paragraph("PRE-RELEASE BUILD / QUALIFICATION MANUAL",
                     ParagraphStyle("t3", parent=WARN, fontSize=12, alignment=1)),
           Spacer(1, 3 * mm),
           Paragraph("The design is defined in the released specifications and BOM. Physical "
                     "fabrication and qualification are NOT complete. Every section carries its "
                     "status; open gates are registered in section 12 and must close on their "
                     "named physical tests before production use. Nothing unverified in this "
                     "document is presented as completed reality.",
                     ParagraphStyle("t4", parent=S, alignment=1, textColor=GREY)),
           Spacer(1, 8 * mm)]
    if (IMAGES / "machine-front.jpg").exists():
        st += [Image(str(IMAGES / "machine-front.jpg"), width=110 * mm, height=42 * mm),
               Paragraph("Machine under construction - reference photo; ratings and dimensions "
                         "are never taken from photos.", CAP)]
    st += [PageBreak()]

    # ------------------------------------------------- document control + TOC
    st += [Paragraph("1. Document control", H2),
           table([["field", "value"],
                  ["document", "release/Cutting_Assembly_Manual.pdf (the one canonical assembly PDF)"],
                  ["revision / date", f"{REVISION} / {BUILD_DATE}"],
                  ["source branch", "completion/2026-10-29 (pinned base 0f28aa3)"],
                  ["BOM authority", "bom/*.csv - purchase/custom lines with dated prices; blanks are not zero cost"],
                  ["spec authority", "shared/*.hpp (single active constants source; legacy scales quarantined)"],
                  ["signal contract", "requirements/signal-table.md (polarity explicit; unassigned channels fail closed)"],
                  ["firmware", "software/machine - host tests green incl. observability/waste/negative-control/recovery groups"],
                  ["analysis", "analysis/* manifests (M27/M28/M31/M33/M36 envelope proofs and budgets)"],
                  ["status vocabulary", "DEFINED / NOT FABRICATED / OPEN GATE (handoff 14); test completion: BLOCKING_PASS / CLOSED"],
                  ["explicitly excluded", "qualification results, delivered-cost claims, safety certification, certified STO"]],
                 widths=[30 * mm, 148 * mm]),
           Paragraph("1.1 Revision history", H3),
           table([["rev", "date", "change"],
                  ["AM-PRE-1", BUILD_DATE, "initial pre-release manual"],
                  ["AM-PRE-2", BUILD_DATE, "expanded work instructions, vector sheets with dimensions and balloons, "
                                           "handling/waste paths, wiring blocks, commissioning, rigor requirements "
                                           "reflected in qualification section"]],
                 widths=[16 * mm, 22 * mm, 140 * mm]),
           Paragraph("1.2 How to read this manual", H3)]
    st += bullets(
        "Each work instruction lists: identity (BOM id), quantity, fastener class/length, "
        "torque status, tooling, inspection point, and a warning where a hazard exists.",
        "Torque values shown as <b>TORQUE PENDING - RELEASE</b> are open gates: they must come "
        "from the released torque table (fastener class and engagement are given so the value "
        "can be derived and recorded at release). Never guess them.",
        "Vector sheets are schematic views generated from released spec geometry with real "
        "coordinates and ballooned BOM identities; they support the instructions but do not "
        "replace the native-CAD release gate.",
        "Where this manual says OPEN GATE, a named record (P-test, TS/TF, or release action) "
        "must exist before the status may change.")
    st += [Paragraph("1.3 Contents", H3),
           table([["section", "content"],
                  ["2", "Safety warnings (construction and operation)"],
                  ["3", "Machine overview, coordinate contract, station map"],
                  ["4", "Build sequence and subassembly index"],
                  ["5", "A: base and frame"],
                  ["6", "B: cutting axes (x2)"],
                  ["7", "C: cutting head and blade tooling"],
                  ["8", "D: feed cassettes and measuring modules"],
                  ["9", "E: payoff and wire path"],
                  ["10", "F: guards; G: cabinet and wiring"],
                  ["11", "Handling and waste paths"],
                  ["12", "Commissioning and qualification program"],
                  ["13", "Maintenance; 14: fault and recovery; 15: spares"],
                  ["16", "Open-gate register"],
                  ["17", "Photographic and vector sheet index"]],
                 widths=[16 * mm, 162 * mm])]
    st += [PageBreak()]

    # ------------------------------------------------------------- safety
    st += [Paragraph("2. Safety warnings", H2),
           Paragraph("Read before any construction, power-up or test work. These warnings do not "
                     "replace the risk assessment and safety validation (P03/P04/P12, OPEN GATE).", WARN)]
    st += [Spacer(1, 2),
           table([["hazard", "who is at risk", "required control"],
                  ["mains and 36/60 V DC buses", "assembler / maintainer",
                   "lock out, prove de-energised before cabinet work; stored energy discharged per M21 records"],
                  ["hardened sharp blades, cam/spring motion", "assembler / operator",
                   "never reach into the head; E-stop removes drive energy but stored energy release must be verified (M37)"],
                  ["payoff spool up to 30 kg plus wire", "assembler",
                   "two-person lift or lifting aid; the friction brake is NOT a safety brake and guarantees no stop"],
                  ["2500 N-class structural loads", "test engineer",
                   "proof loads are machine loads, never body loads; exclusion zone during any powered load test"],
                  ["ejected stubs/slugs", "operator",
                   "waste chute and bin are handling channels, not safety devices; clear only with drives disabled"],
                  ["electrical diagrams herein", "assembler",
                   "functional reference only; the electrical release (G32) governs conductor sizes, fuses and torque"],
                  ["unqualified machine", "all",
                   "production use prohibited until section 16 open gates close on signed records"]],
                 widths=[42 * mm, 32 * mm, 104 * mm])]
    st += [PageBreak()]

    # ------------------------------------------------------------- overview
    st += [Paragraph("3. Machine overview and station map", H2),
           Paragraph("Coordinate contract: X = wire travel, Y = opposing cutting strokes, Z = up; "
                     "cut plane X0; wire at Y0/Z112; +X toward the piece/eject side. Units mm, "
                     "N, N.m, s. The elevation below shows every real X station used by the "
                     "sensing/handling contract.", S),
           View(sheet_machine_elevation, height=96 * mm),
           Paragraph("Key geometry: stock-side measuring wheels W0/W1 (X=-385/-205) can never "
                     "observe a severed piece; piece motion is observed only at the downstream "
                     "station D (candidate X=106.75, mechanical fit M02 OPEN GATE); the right "
                     "cassette cannot propel a severed body shorter than 140 mm; the eject's "
                     "unobserved remainder is covered by the discharge present->clear evidence "
                     "(analysis/transport-geometry: 5184-cell envelope proof).", S)]
    st += [PageBreak()]

    # --------------------------------------------------------- build sequence
    st += [Paragraph("4. Build sequence and subassembly index", H2),
           table([["order", "subassembly", "section", "depends on", "status"],
                  ["A", "base and frame", "5", "-", "DEFINED / NOT FABRICATED"],
                  ["B", "cutting axes x2", "6", "A", "DEFINED / NOT FABRICATED (G37 load gate)"],
                  ["C", "head and blade tooling", "7", "B", "DEFINED / NOT FABRICATED"],
                  ["D", "feed cassettes + measuring modules", "8", "A", "DEFINED / NOT FABRICATED (D-station fit gate)"],
                  ["E", "payoff and wire path", "9", "A", "DEFINED / NOT FABRICATED"],
                  ["F", "guards", "10", "B,C,D,E", "DEFINED / NOT FABRICATED (G34)"],
                  ["G", "cabinet and wiring", "10", "-", "DEFINED / electrical release OPEN"],
                  ["H", "commissioning + qualification", "12", "all", "OPEN GATE program"]],
                 widths=[12 * mm, 52 * mm, 14 * mm, 20 * mm, 80 * mm]),
           Paragraph("Rule: no subassembly may proceed past its section while any warning or "
                     "OPEN GATE in that section is unresolved (handoff 14/16.4).", SB)]

    # --------------------------------------------------------------- base
    st += [Paragraph("5. A - Base and frame", H2)]
    st += [table([["step", "work instruction", "fasteners / torque", "tools", "inspection", "status"],
                  ["A-1", "Set H001 ground datum plate (180x1040x20) on the prepared floor; "
                          "reference the cut-plane datum mark.",
                   "4x M12 floor anchors (site) - TORQUE PENDING - RELEASE",
                   "level, torque wrench", "flatness and level record", "DEFINED"],
                  ["A-2", "Mount H002 risers (110x355x20) to the datum stage grid (46/100).",
                   "per drawing - TORQUE PENDING - RELEASE", "M8/M10 wrenches",
                   "stage grid verified; riser parallelism", "DEFINED"],
                  ["A-3", "Set the welded frame onto the risers; align to the X-station map "
                          "(cut plane datum, wheel and cassette stations).",
                   "M10x35 class 8.8 bed-to-frame x8 - TORQUE PENDING - RELEASE",
                   "straight edge, level", "station positions vs section 3 map", "DEFINED"],
                  ["A-4", "Mount the cabinet plinth and route main cable entries.",
                   "per electrical release", "electrician tools", "entry sealing; G32 open", "OPEN GATE"]],
                 widths=[10 * mm, 66 * mm, 38 * mm, 22 * mm, 28 * mm, 14 * mm])]
    st += [PageBreak()]

    # --------------------------------------------------------- cutting axes
    st += [Paragraph("6. B - Cutting axes (2 identical)", H2),
           View(sheet_axis_exploded, height=88 * mm)]
    st += [Paragraph("WARNING - proof loading (2500 N axial / 750 N transverse / 375 N vertical, "
                     "double) is a controlled validation procedure (P05), not an assembly step. "
                     "The support rating class (4900 N) sits below the 5000 N proof load: this "
                     "OPEN GATE (G37) is resolved by engineering change, never by rounding.", WARN),
           table([["step", "work instruction", "fasteners / torque", "tools", "inspection", "status"],
                  ["B-1", "Mount H005 base plate (20x86x440) - rail seats milled, 6x 6.6 grid 46/100.",
                   "6x ISO4762 M5x16 cl.10.9 class (HF0-B class) + ISO7092 M5 washers - TORQUE PENDING - RELEASE",
                   "M5 hex key / torque wrench", "plate flatness; seat contact", "DEFINED"],
                  ["B-2", "Fit AX-R01 rail with 2x HGH20CA blocks (2 sets per axis).",
                   "M5 rail screws at 60 pitch - TORQUE PENDING - RELEASE",
                   "M5 hex key", "rail straightness; block preload class", "DEFINED"],
                  ["B-3", "Fit AX-S01 ballscrew (SFU1610 C7, 500 mm, 10 mm lead) with DSG16h "
                          "coupling at the drive end; journal ends 8 h7x18.",
                   "nut flange screws per manufacturer - TORQUE PENDING - RELEASE",
                   "hex keys, dial indicator", "runout; end machining conformance (P01-T3)", "DEFINED"],
                  ["B-4", "Install BK12/BF12 supports on their lands; align screw to rail.",
                   "M5 class screws - TORQUE PENDING - RELEASE", "dial indicator",
                   "support alignment; screw free rotation", "DEFINED"],
                  ["B-5", "Fit H006 carriage plate (60x75 pad at 46) to the blocks.",
                   "4x ISO4762 M5x16 cl.10.9 (HF0-B) + ISO7092 M5 washers, 8.5 mm nominal engagement - TORQUE PENDING - RELEASE",
                   "M5 hex key", "49.97 F0 register; PCD70 M5", "DEFINED"],
                  ["B-6", "Fit motor kit DR-01 (34E1K-85 + CL86T-V41) via H003 adapter (15 mm) "
                          "and CP-01 coupling (16 mm hub fit on 18 mm exposed shaft).",
                   "8x ISO4762 M5x25 cl.10.9 (HM-B) + ISO7092 M5 200HV washers (HM-W, 5.3x9x1, "
                   "reduced OD 9 mm for the 9.5 mm counterbore); 15 mm adapter thread engagement - TORQUE PENDING - RELEASE",
                   "M5 hex key, feeler", "134.1 mm rear envelope clearance; coupling alignment (P01-T5)", "DEFINED"],
                  ["B-7", "Repeat B-1..B-6 for the second axis; label axis identity "
                          "(blade_front / blade_rear) on the base plate.", "-", "-", "both axes identical", "DEFINED"]],
                 widths=[10 * mm, 55 * mm, 48 * mm, 18 * mm, 33 * mm, 14 * mm])]
    st += [PageBreak()]

    # ----------------------------------------------------------- head/tooling
    st += [Paragraph("7. C - Cutting head and blade tooling", H2),
           View(sheet_head_assembly, height=76 * mm),
           table([["step", "work instruction", "fasteners / torque", "tools", "inspection", "status"],
                  ["C-1", "Mount H004 holder (42CrMo4+QT, 650 MPa min yield) to the carriage; "
                          "the 70 mm backbone is centred on the screw/load line (R3).",
                   "per drawing - TORQUE PENDING - RELEASE", "M5/M6 keys",
                   "holder centring; nose pocket only machined", "DEFINED"],
                  ["C-2", "Fit H007 backing plates (30x46x8) and shims; record every shim.",
                   "TORQUE PENDING - RELEASE", "shim kit, feeler",
                   "shim record filed (G22)", "DEFINED"],
                  ["C-3", "Insert T001 matched 90 deg V blade pair (49x38x3); orientation: "
                          "V-apex faces the wire centreline.", "blade clamps - TORQUE PENDING - RELEASE",
                   "microscope 40x", "edge radius; apex alignment", "DEFINED"],
                  ["C-4", "Set loaded blade face gap 0.020 mm nominal at the wire using the "
                          "shim procedure; execute on both axes.",
                   "-", "feeler / interferometry", "gap record per axis (P07-T6)", "OPEN GATE interval"],
                  ["C-5", "Record blade serials and cut-count baseline; set the wear "
                          "inspection plan.", "-", "-", "spares and limit plan (P07-T4)", "OPEN GATE"]],
                 widths=[10 * mm, 60 * mm, 36 * mm, 22 * mm, 33 * mm, 17 * mm])]
    st += [PageBreak()]

    # --------------------------------------------------- feed + measurement
    st += [Paragraph("8. D - Feed cassettes and measuring modules", H2),
           View(sheet_feed_cassette, height=76 * mm),
           table([["step", "work instruction", "fasteners / torque", "tools", "inspection", "status"],
                  ["D-1", "Assemble F-series cassette (housing F102/F105, shaft F101 x2, "
                          "rollers, springs; 46/100 or 64/47 centres per variant).",
                   "TR-FB01 ISO4762 M5x16 cl.10.9 x8 (roller endplate); TR-FB02 ISO4762 M4x20 "
                   "cl.10.9 x16 + TR-FW02 ISO7092 M4 washers (F112 front module); TR-FD02 ISO8734 "
                   "3m6x20 dowels x16; TR-FB03 ISO4762 M3x8 cl.8.8 x32 (F103 cover) - TORQUE PENDING - RELEASE",
                   "hex keys, press", "roller free rotation; endplate parallelism", "DEFINED"],
                  ["D-2", "Fit feed motor TR-M01 (24HS40-5004D-E1000, 10 mm shaft) and the "
                          "HTD5M belt with 20T pulley; set tension 180-200 N (target 190) at "
                          "BOTH stations.", "pulley clamp screws - TORQUE PENDING - RELEASE",
                   "tension meter", "both station tensions recorded (P06-T3)", "DEFINED"],
                  ["D-3", "Mount measuring wheel modules W0/W1 (ifm E60137, 63.66 mm OD) with "
                          "RU3100 encoders on the stock side (X=-385/-205).",
                   "ME-FB01 ISO4762 M4x12 cl.8.8 x6 (M201 encoder plate; ifm M4x0.7x6 taps, "
                   "6.0 mm nominal engagement; encoder thread torque limit PENDING)",
                   "M4 hex key", "wheel force 6-19 N each (P06-T2)", "DEFINED"],
                  ["D-4", "Downstream piece station D (candidate X=106.75): do NOT install "
                          "until the M02 fit study closes (63.66 mm wheel/encoder/housing among "
                          "the nips at 75.5/138).", "-", "-",
                   "M02 fit gate; Phase-H pin assignment (analysis/phase-h-io)", "OPEN GATE"],
                  ["D-5", "Route encoder cabling to GP8-11; leave station-D and handling "
                          "channels unassigned until Phase H (they fail closed in firmware).",
                   "-", "cable ties", "signal-table conformance (P04-T4)", "OPEN GATE"]],
                 widths=[10 * mm, 55 * mm, 50 * mm, 16 * mm, 30 * mm, 17 * mm])]
    st += [PageBreak()]

    # ---------------------------------------------------------------- payoff
    st += [Paragraph("9. E - Payoff and wire path", H2),
           View(sheet_payoff, height=82 * mm),
           table([["step", "work instruction", "fasteners / torque", "tools", "inspection", "status"],
                  ["E-1", "Set P003 base (430x560x8) level; anchor with 4x M12.",
                   "4x M12 anchors - TORQUE PENDING - RELEASE", "level, wrench",
                   "base level record", "DEFINED"],
                  ["E-2", "Fit P004/P005 support posts with SKF 6204-2Z/C3 bearings in the "
                          "47 H7 pockets (14 deep).", "M8 base bolts - TORQUE PENDING - RELEASE",
                   "bearing press tool", "pocket H7; bearing seating (P01-T1 class)", "DEFINED"],
                  ["E-3", "Insert P002 spindle (C45 20 h6 L660); fit M20x1.5 retention nuts "
                          "with tab washers.", "M20x1.5 KM-class nuts (PY-R01) - TORQUE PENDING - RELEASE",
                   "pin wrench", "spindle free rotation; axial float limit", "DEFINED"],
                  ["E-4", "Fit P008 drive boss (keyseat 6 JS9) welded to the LEFT flange "
                          "inside the hub tube BEFORE spool assembly.",
                   "weld per fabrication procedure", "welding shop",
                   "weld inspection; key fit", "DEFINED"],
                  ["E-5", "Assemble P001 spool (2x S235JR 400x3 flanges + 60x2.5 ERW hub 160) "
                          "and mount with P006 disc + P007 caliper brake.",
                   "set screws / caliper adjuster M8 DIN933 - TORQUE PENDING - RELEASE",
                   "hex keys", "brake effective ONLY with P008 fitted", "DEFINED"],
                  ["E-6", "Weigh the full spool per lot (wire included); verify <=30 kg and "
                          "record mass.", "-", "scale",
                   "lot mass record (docs/26 s4); two-person lift", "DEFINED"]],
                 widths=[10 * mm, 58 * mm, 44 * mm, 20 * mm, 30 * mm, 16 * mm])]
    st += [PageBreak()]

    # ------------------------------------------------------ guards + cabinet
    st += [Paragraph("10. F - Guards; G - Cabinet and wiring", H2),
           Paragraph("F: Guards", H3),
           table([["step", "work instruction", "fasteners / torque", "inspection", "status"],
                  ["F-1", "Fit guards over the head, cassettes, payoff and takeaway per the "
                          "layout; include reel/buffer/conveyor hazards.",
                   "per drawing - TORQUE PENDING - RELEASE",
                   "opening/distance check vs reachable hazards (P04-T2)", "OPEN GATE G34"],
                  ["F-2", "Fit guard switches (SLC-F-024-20/20-R4 class) and route to the "
                          "XPSUAF13AP dual channel.", "TORQUE PENDING - RELEASE",
                   "dual-channel validation (P04-T1)", "OPEN GATE G35"]],
                 widths=[10 * mm, 78 * mm, 34 * mm, 40 * mm, 16 * mm]),
           Paragraph("G: Cabinet and wiring", H3),
           View(sheet_cabinet_layout, height=72 * mm)]
    st += [View(sheet_wiring_blocks, height=60 * mm),
           table([["step", "work instruction", "inspection", "status"],
                  ["G-1", "Build the cabinet per the layout: 230 V in -> protection -> 24 V PSU "
                          "-> isolated 60 V PSUs -> drivers -> core. No bus paralleling.",
                   "point-to-point against requirements/signal-table.md", "OPEN GATE G32"],
                  ["G-2", "Safety chain: E-stop (SIRIUS ACT 3SU1851) + guard loop -> XPSUAF13AP "
                          "-> ENA common (GP17). Dual channel all the way.",
                   "single-fault tolerance test (P04-T1)", "OPEN GATE G35"],
                  ["G-3", "Field inputs via PLC-RSC 24DC/21 couplers; wetting current must "
                          "exceed relay minima (the old 57.9 uA defect class must not recur).",
                   "wetting measurement (P03-T5)", "DEFINED"],
                  ["G-4", "ALM lines to GP18-21; encoders to GP8-11 via AM26LV32E receivers; "
                          "USB-CDC is optional and never required for motion.",
                   "continuity + polarity audit (P04-T4)", "DEFINED"],
                  ["G-5", "Leave part_clear / waste_clear / waste_sweep / station-D unassigned "
                          "until Phase H; firmware fails closed meanwhile.",
                   "verify fail-closed behaviour on power-up", "OPEN GATE Phase-H"]],
                 widths=[10 * mm, 88 * mm, 52 * mm, 28 * mm])]
    st += [PageBreak()]

    # ------------------------------------------------- handling and waste
    st += [Paragraph("11. Handling and waste paths", H2),
           View(sheet_handling_path, height=72 * mm),
           Paragraph("The three body classes have different, mandatory evidence chains "
                     "(firmware enforces them in production):", S)]
    st += bullets(
        "<b>Prime stub (10 mm):</b> severed at the plane; it cannot reach the nearest right "
        "nip (75.5 mm) and is never propelled by feed motion. The constrained sweep "
        "(waste_sweep) must produce a chute transit (waste_clear present->clear) before any "
        "further feed or blade close.",
        "<b>Strip slugs (3-25 mm):</b> released at blade open after each score+separation; "
        "same sweep and chute-transit requirement at LeadOpen/TailOpen.",
        "<b>Finished part (200-3000 mm):</b> driven by the right cassette only while its tail "
        "is inside the far nip (X140); then the takeaway (>=130 mm slide) clears the discharge "
        "zone X230-270. Acceptance requires the discharge present->clear pair inside the "
        "ejection observation window - never commanded steps alone.",
        "<b>Interrupted in-flight body:</b> classified by real length at the prime re-cut - "
        "short goes to the sweep, long is scrap-discharged through the takeaway and counted "
        "scrap (recovery drill P08-T6).")
    st += [Paragraph("Blocked sweep, stuck sensors and early/late clears all fault and "
                     "quarantine - they never produce accepted counts (firmware regression "
                     "groups waste / observability / recovery).", SB), PageBreak()]

    # -------------------------------------------------------- commissioning
    st += [Paragraph("12. Commissioning and qualification program", H2),
           Paragraph("12.1 Commissioning sequence", H3)]
    st += numbered(
        "Flash the released firmware image; record its SHA-256 in the machine log. The factory "
        "test runs from a separate UF2 and must exercise every I/O channel.",
        "Power up with drives disabled; verify fail-closed handling channels report NOT clear.",
        "Home both blade axes; route wire through both cassettes (threading per section 3).",
        "Service fixture mode (`starz-ctl ... fixture on`) is for engineering runs only: it "
        "never emits accepted counts or persistent approval. Leave it OFF for production.",
        "Run the commissioning polarity moves for all four axes (station D when fitted); "
        "calibrate W0/W1/D (starz-ctl calib <w0> <w1> <piece> after the length record).",
        "Approve the retained recipe over the host link (Msg::Approve) - approval is keyed to "
        "calibration + recipe + firmware identity and is cleared by any change.",
        "Production START requires the approval; start with commissioning speed (0.05 m/s "
        "class) and the handling verification ON (default).",
        "Run the qualification program below; only signed records change any status.")
    st += [Paragraph("12.2 Qualification program (P00-P15 families)", H3),
           table([["family", "scope", "records"],
                  ["P00 receiving", "identity, first article, evidence/price reconciliation", "tests/P00-P15/records/P00-T*.json"],
                  ["P01 fits", "interference pairs, runout, torques, alignment", "P01-T*.json"],
                  ["P02/P03/P04 electrical", "protection curves, power components, safety wiring", "P02-T*..P04-T*.json"],
                  ["P05 loads", "proof loads and post-proof metrology", "P05-T*.json"],
                  ["P06 transport", "nip/wheel forces, belt tension, loaded radius", "P06-T*.json"],
                  ["P07 quality", "cut/strip coupons, blade life, face gap", "P07-T*.json"],
                  ["P08 handling", "stub/slug/output evidence chains", "P08-T*.json"],
                  ["P09 dynamics", "tooth-pass, Campbell, FRF, damping", "P09-T*.json"],
                  ["P10 thermal", "18 TS steady + 17 TF fault (tests/P10-thermal/)", "TS*.json / TF*.json"],
                  ["P11 electrical system", "regen decay, bleed timing, long-run behaviour", "P11-T*.json"],
                  ["P12 safety validation", "PLr/SIL, ISO 13849-2, engineer sign-off", "P12-T*.json"],
                  ["P13 accuracy", "length/strip capability, gauge R&R, budgets", "P13-T*.json"],
                  ["P14 endurance", "duty blocks, wear limits, service intervals", "P14-T*.json"],
                  ["P15 integration", "independent build, collisions, final inspection", "P15-T*.json"]],
                 widths=[30 * mm, 96 * mm, 52 * mm]),
           Paragraph("All records ship executed=false. Rigor requirements (traceability, |e|+U<=T "
                     "conformity, sample sizes, predeclared analysis) are stated in "
                     "tests/P00-P15/procedures.md and apply to every test.", SB)]
    st += [PageBreak()]

    # ------------------------------------------------------------ service
    st += [Paragraph("13. Maintenance (provisional until P14/TS evidence)", H2),
           table([["item", "interval (provisional)", "limit / action", "record"],
                  ["ventilation filters (2 paths)", "inspect each shift; replace per TS evidence",
                   "installed flow below service limit triggers change", "P10 + TS01-TS18"],
                  ["blades T001", "inspect each shift; replace per P07-T4 limit",
                   "edge radius/wear limit from coupons", "P07-T4"],
                  ["measuring wheels W0/W1/D", "weekly",
                   "tread wear and calibration drift check", "P06-T2/T5"],
                  ["nips and guides", "weekly", "50-75 N nip force check", "P06-T1"],
                  ["belt tension", "monthly + after any drive work", "180-200 N at both stations", "P06-T3"],
                  ["payoff brake pads", "monthly", "friction pad wear", "P14-T2"],
                  ["spindle bearings", "per P14 duty evidence", "lubrication interval to be established", "P14-T3"],
                  ["safety functions", "per P12 proof-test interval", "E-stop/guard validation repeat", "P12-T4"]],
                 widths=[42 * mm, 48 * mm, 56 * mm, 32 * mm])]
    st += [Paragraph("14. Fault and recovery (operator)", H2),
           table([["fault", "meaning", "operator action", "do NOT"],
                  ["red LED steady", "latched fault; drives disabled",
                   "clear cause; hold RESET 0.5 s; rehome", "force motion without rehoming"],
                  ["no_wire / length_slip", "material motion mismatch",
                   "inspect path and wheels; rethread; RESET", "accept the piece in the machine"],
                  ["part_not_cleared", "discharge evidence missing (quarantine)",
                   "remove/verify part at the takeaway; log scrap", "count the piece as accepted"],
                  ["waste_not_cleared", "stub/slug did not transit the chute",
                   "clear sweep chute and pocket; RESET", "feed over a trapped body"],
                  ["wire_geometry", "planned move/score rejected by the wire model",
                   "rethread and rehome; report the case", "bypass the check"],
                  ["power loss mid-piece", "in-flight part uncertain",
                   "on restart: rehome + rethread; prime scrap-discharges leftovers",
                   "resume as if coordinates were truth"]],
                 widths=[30 * mm, 42 * mm, 62 * mm, 44 * mm])]
    st += [PageBreak()]

    st += [Paragraph("15. Spare parts (initial)", H2),
           table([["part", "identity", "qty", "status"],
                  ["blades", "T001 matched 90 deg V pair 49x38x3", "2 pairs", "DEFINED / NOT FABRICATED"],
                  ["filters", "Pfannenberg 11666102055 / 11760002055 candidates", "1 set", "OPEN GATE TS evidence"],
                  ["measuring wheels", "ifm E60137", "1", "DEFINED"],
                  ["encoders", "ifm RU3100", "1", "DEFINED"],
                  ["belt", "HTBN225S5M-250", "1", "DEFINED"],
                  ["couplings", "R+W MK2/100/50/8/14", "1", "DEFINED"],
                  ["fuses/relays", "per electrical release", "-", "OPEN GATE G32"],
                  ["brake pads", "docs/26 P007 friction pads", "1 set", "DEFINED"]],
                 widths=[30 * mm, 88 * mm, 22 * mm, 38 * mm])]

    # ---------------------------------------------------------- open gates
    st += [Paragraph("16. Open-gate register (production prohibited until closed)", H2),
           table([["gate", "what is owed", "record / owner"],
                  ["G37 axis loads", "revalidate acceptance loads vs the frozen 10 mm-lead screw; "
                                     "resolve 4900 N < 5000 N proof contradiction by change", "P05-T6"],
                  ["G16 fits", "3-6 um interference matched pairs + bearing clearance envelope", "P01-T1 / P13"],
                  ["G18 shaft thermal", "shaft rise <=20 K above housing (stored 29.66 K case fails)", "P10"],
                  ["G22 face gap", "0.020 mm loaded interval derived from cut-quality data", "P07-T6"],
                  ["G29-G30 ventilation", "installed filtered flow + thermal margins; 18 TS + 17 TF", "P10 program"],
                  ["G32 protection", "exact fuse/breaker coordination; conductor/terminal limits", "P02 / electrical release"],
                  ["G33 regen/bleed", "per-branch absorption/bleed with measured bus decay", "P11-T1/T2"],
                  ["G35 safety", "risk-derived PLr/SIL; dual-channel validation; competent engineer", "P12 sign-off"],
                  ["M02 D-station fit", "wheel/encoder/housing fit in X38-140 among nips 75.5/138", "fit study + P01"],
                  ["Phase-H I/O", "assign part_clear / waste_clear / waste_sweep / station-D pins",
                   "analysis/phase-h-io option study"],
                  ["G39 RFQs", "send the drafted enquiries (7 exact RFQs)", "procurement/rfq-drafts.md"],
                  ["G47 cost", "complete landed cost; 70 % like-for-like comparison", "analysis/cost-basis (NOT ESTABLISHED)"],
                  ["P15 independence", "independent builder completes this manual without choices", "P15-T1"]],
                 widths=[34 * mm, 110 * mm, 34 * mm])]

    # ------------------------------------------------------ photos + sheets
    st += [PageBreak(), Paragraph("17. Photographic and vector sheet index", H2),
           Paragraph("Reference photos from the machine under construction. They establish no "
                     "ratings, dimensions or qualification.", S)]
    for img, cap in [("machine-rear.jpg", "rear view (reference)"),
                     ("payoff-and-buffer.jpg", "payoff and buffer (reference)")]:
        p = IMAGES / img
        if p.exists():
            st += [Image(str(p), width=120 * mm, height=44 * mm), Paragraph(cap, CAP), Spacer(1, 3)]
    st += [Paragraph("Vector sheets (drawn from released spec geometry; real coordinates):", SB),
           *bullets("sheet 1 - machine side elevation with the station dimension chain (section 3)",
                   "sheet 2 - AX-01 cut-axis exploded schematic (section 6)",
                   "sheet 3 - cutting head front view + blade detail (section 7)",
                   "sheet 4 - feed cassette schematic + belt side view (section 8)",
                   "sheet 5 - payoff exploded schematic (section 9)",
                   "sheet 6 - cabinet layout and wiring blocks (section 10)",
                   "sheet 7 - handling and waste paths (section 11)"),
           Paragraph("End of manual. Status changes require signed records referenced in "
                     "section 16.", SB)]

    doc.build(st)

    inputs = {
        "bom/selected-cutting-head.csv": sha256(ROOT / "bom" / "selected-cutting-head.csv")[:16],
        "bom/selected-transport-and-cabinet.csv": sha256(ROOT / "bom" / "selected-transport-and-cabinet.csv")[:16],
        "shared/controls_spec.hpp": sha256(ROOT / "shared" / "controls_spec.hpp")[:16],
        "requirements/signal-table.md": sha256(ROOT / "requirements" / "signal-table.md")[:16],
    }
    build_record = {
        "artifact": "release/Cutting_Assembly_Manual.pdf",
        "revision": REVISION,
        "build_date": BUILD_DATE,
        "builder": "tools/build_assembly_manual.py (reportlab)",
        "embedded_input_hashes_16": inputs,
        "declared_status": "PRE-RELEASE BUILD/QUALIFICATION MANUAL",
    }
    (ROOT / "release" / "assembly-manual-build.json").write_text(
        json.dumps(build_record, indent=2, sort_keys=True) + "\n", encoding="utf-8")
    print(f"wrote {OUT.relative_to(ROOT)} ({OUT.stat().st_size} bytes)")


if __name__ == "__main__":
    build()
