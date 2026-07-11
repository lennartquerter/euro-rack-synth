#!/usr/bin/env python3
"""Generate the sequencer bank (sequences.c/h) from sequences/sequences.md.

Usage:
    python3 tools/gen_sequences.py            # default paths, run from anywhere
    python3 tools/gen_sequences.py --md X --out-c Y --out-h Z

Grammar of sequences.md (see also the header of that file):
  # Bank A ...            starts a bank section (A = lead, B = bass)
  ## <name>               starts a sequence inside the current bank
  <bar line>              32 tokens (8 beats x 4 sub-steps), '|' is a visual
                          beat separator and is ignored; 4 bar lines per
                          sequence
  %                       repeat the previous bar
  %1..%4                  repeat bar N of this sequence
  > ...                   comment line (ignored)
  blank lines             ignored; anything before the first bank is preamble

Tokens (one per sub-step, 4 PPQN):
  .        rest (gate off)
  c2       note + octave: gate on, retrigger. Letters a-g, accidental # or b.
  c2~      tie: gate held (no retrigger), pitch c2 (legato pitch change ok)
  ~        tie continuing the previous pitch
  !        accent suffix (reserved flag), e.g. c2! or c2~!

Counts are never hard-coded downstream: the firmware reads SEQ_bank_*_count.
"""

import argparse
import re
import sys
from pathlib import Path

# single source of truth for the phrase structure ("32 beats" decision)
BARS_PER_PHRASE = 4
BEATS_PER_BAR = 8
SUBSTEPS_PER_BEAT = 4  # 4 PPQN clock: one token per pulse
BAR_STEPS = BEATS_PER_BAR * SUBSTEPS_PER_BEAT
PHRASE_STEPS = BARS_PER_PHRASE * BAR_STEPS

FLAG_GATE = 0x01
FLAG_TIE = 0x02
FLAG_ACCENT = 0x04

REST_NOTE = -1
NOTE_MAX = 120  # 0-10V at 1V/oct

NOTE_BASE = {"c": 0, "d": 2, "e": 4, "f": 5, "g": 7, "a": 9, "b": 11}

BANK_RE = re.compile(r"^#\s*bank\s+([ab])\b", re.IGNORECASE)
SEQ_RE = re.compile(r"^##\s+(.+?)\s*$")
BAR_REPEAT_RE = re.compile(r"^%([1-9])?$")
NOTE_RE = re.compile(r"^([a-g])([#b]?)(\d)(~)?(!)?$", re.IGNORECASE)
TIE_RE = re.compile(r"^(~)(!)?$")


class ParseError(Exception):
    pass


def parse_token(token, last_note, where):
    """Return (note, flags, last_note)."""
    if token == ".":
        return REST_NOTE, 0, last_note

    m = TIE_RE.match(token)
    if m:
        if last_note is None:
            raise ParseError(f"{where}: bare '~' with no previous note")
        flags = FLAG_GATE | FLAG_TIE | (FLAG_ACCENT if m.group(2) else 0)
        return last_note, flags, last_note

    m = NOTE_RE.match(token)
    if not m:
        raise ParseError(f"{where}: bad token '{token}'")
    letter, accidental, octave, tie, accent = m.groups()
    note = NOTE_BASE[letter.lower()] + 12 * int(octave)
    if accidental == "#":
        note += 1
    elif accidental == "b":
        note -= 1
    if not 0 <= note <= NOTE_MAX:
        raise ParseError(f"{where}: note '{token}' out of range 0..{NOTE_MAX}")
    flags = FLAG_GATE
    if tie:
        flags |= FLAG_TIE
    if accent:
        flags |= FLAG_ACCENT
    return note, flags, note


def parse_bar(line, bars, last_note, where):
    """Return (steps, last_note) for one bar line."""
    m = BAR_REPEAT_RE.match(line)
    if m:
        if m.group(1) is None:
            if not bars:
                raise ParseError(f"{where}: '%' with no previous bar")
            steps = bars[-1]
        else:
            n = int(m.group(1))
            if n > len(bars):
                raise ParseError(f"{where}: '%{n}' but only {len(bars)} bars so far")
            steps = bars[n - 1]
        note_steps = [s for s in steps if s[0] != REST_NOTE]
        if note_steps:
            last_note = note_steps[-1][0]
        return list(steps), last_note

    tokens = line.replace("|", " ").split()
    if len(tokens) != BAR_STEPS:
        raise ParseError(
            f"{where}: expected {BAR_STEPS} tokens "
            f"({BEATS_PER_BAR} beats x {SUBSTEPS_PER_BEAT}), got {len(tokens)}"
        )
    steps = []
    for token in tokens:
        note, flags, last_note = parse_token(token, last_note, where)
        steps.append((note, flags))
    return steps, last_note


def parse_md(path):
    banks = {"a": [], "b": []}
    bank = None
    seq_name = None
    seq_bars = []
    last_note = None

    def finish_sequence(where):
        nonlocal seq_name, seq_bars, last_note
        if seq_name is None:
            return
        if len(seq_bars) != BARS_PER_PHRASE:
            raise ParseError(
                f"{where}: sequence '{seq_name}' has {len(seq_bars)} bars, "
                f"expected {BARS_PER_PHRASE}"
            )
        steps = [step for bar in seq_bars for step in bar]
        banks[bank].append((seq_name, steps))
        seq_name = None
        seq_bars = []
        last_note = None

    lines = path.read_text().split("\n")
    for lineno, raw in enumerate(lines, 1):
        line = raw.strip()
        where = f"{path.name}:{lineno}"
        if not line or line.startswith(">") or line.startswith("<!--"):
            continue

        m = BANK_RE.match(line)
        if m:
            finish_sequence(where)
            bank = m.group(1).lower()
            continue

        m = SEQ_RE.match(line)
        if m:
            if bank is None:
                raise ParseError(f"{where}: sequence heading before any bank")
            finish_sequence(where)
            seq_name = m.group(1)
            continue

        if line.startswith("#"):
            if bank is None:
                continue  # preamble prose/title
            raise ParseError(f"{where}: unexpected heading '{line}'")

        if bank is None:
            continue  # preamble prose
        if seq_name is None:
            raise ParseError(f"{where}: bar line outside a sequence")
        if len(seq_bars) >= BARS_PER_PHRASE:
            raise ParseError(
                f"{where}: sequence '{seq_name}' already has "
                f"{BARS_PER_PHRASE} bars"
            )
        steps, last_note = parse_bar(line, seq_bars, last_note, where)
        seq_bars.append(steps)

    finish_sequence(f"{path.name}:EOF")
    return banks


def emit_steps(name, steps):
    out = [f"static const struct SEQ_step {name}[SEQ_PHRASE_STEPS] = {{"]
    for i in range(0, len(steps), 8):
        chunk = ", ".join(f"{{{n}, 0x{f:02X}}}" for n, f in steps[i : i + 8])
        out.append(f"    {chunk},")
    out.append("};")
    return "\n".join(out)


def c_escape(text):
    return text.replace("\\", "\\\\").replace('"', '\\"')


def generate(banks, md_name):
    warning = (
        f"/* AUTO-GENERATED by tools/gen_sequences.py from {md_name}"
        " — DO NOT EDIT */"
    )

    h = f"""{warning}

#ifndef __SEQUENCES_H
#define __SEQUENCES_H

#include <stdint.h>

/* phrase structure — single source of truth is gen_sequences.py */
#define SEQ_BARS_PER_PHRASE {BARS_PER_PHRASE}
#define SEQ_BEATS_PER_BAR {BEATS_PER_BAR}
#define SEQ_SUBSTEPS_PER_BEAT {SUBSTEPS_PER_BEAT} /* 4 PPQN clock input */
#define SEQ_BAR_STEPS (SEQ_BEATS_PER_BAR * SEQ_SUBSTEPS_PER_BEAT)
#define SEQ_PHRASE_STEPS (SEQ_BARS_PER_PHRASE * SEQ_BAR_STEPS)

#define SEQ_FLAG_GATE 0x01   /* note plays on this sub-step */
#define SEQ_FLAG_TIE 0x02    /* gate held, no retrigger (legato) */
#define SEQ_FLAG_ACCENT 0x04 /* reserved */

#define SEQ_STEP_REST (-1)

struct SEQ_step
{{
    int8_t note;   /* semitone from 0V C, SEQ_STEP_REST when silent */
    uint8_t flags; /* SEQ_FLAG_* */
}};

struct SEQ_sequence
{{
    const char* name;
    uint16_t step_count;
    const struct SEQ_step* steps;
}};

/* bank sizes are data, never hard-code them downstream */
extern const struct SEQ_sequence SEQ_bank_a[]; /* lead */
extern const uint16_t SEQ_bank_a_count;
extern const struct SEQ_sequence SEQ_bank_b[]; /* bass */
extern const uint16_t SEQ_bank_b_count;

#endif
"""

    parts = [warning, "", '#include "app/sequences.h"', ""]
    for bank in ("a", "b"):
        for idx, (name, steps) in enumerate(banks[bank]):
            parts.append(f"/* {bank.upper()}{idx:02d}: {name} */")
            parts.append(emit_steps(f"seq_{bank}{idx:02d}_steps", steps))
            parts.append("")
    for bank in ("a", "b"):
        parts.append(f"const struct SEQ_sequence SEQ_bank_{bank}[] = {{")
        for idx, (name, _steps) in enumerate(banks[bank]):
            parts.append(
                f'    {{"{c_escape(name)}", SEQ_PHRASE_STEPS,'
                f" seq_{bank}{idx:02d}_steps}},"
            )
        parts.append("};")
        parts.append(
            f"const uint16_t SEQ_bank_{bank}_count ="
            f" sizeof(SEQ_bank_{bank}) / sizeof(SEQ_bank_{bank}[0]);"
        )
        parts.append("")
    c = "\n".join(parts)
    return h, c


def main():
    root = Path(__file__).resolve().parent.parent
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("--md", default=root / "sequences" / "sequences.md")
    parser.add_argument("--out-c", default=root / "Core" / "Src" / "app" / "sequences.c")
    parser.add_argument("--out-h", default=root / "Core" / "Inc" / "app" / "sequences.h")
    args = parser.parse_args()

    md_path = Path(args.md)
    try:
        banks = parse_md(md_path)
    except ParseError as error:
        print(f"error: {error}", file=sys.stderr)
        return 1

    for bank in ("a", "b"):
        if not banks[bank]:
            print(f"error: bank {bank.upper()} has no sequences", file=sys.stderr)
            return 1

    h, c = generate(banks, md_path.name)
    Path(args.out_h).write_text(h)
    Path(args.out_c).write_text(c)
    print(
        f"generated {Path(args.out_c).name} / {Path(args.out_h).name}: "
        f"bank A = {len(banks['a'])}, bank B = {len(banks['b'])} sequences, "
        f"{PHRASE_STEPS} steps each"
    )
    return 0


if __name__ == "__main__":
    sys.exit(main())
