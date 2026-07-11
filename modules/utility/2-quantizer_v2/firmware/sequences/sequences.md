# Sequencer bank — quantizer v2 "flip side"

Authored sequence bank, compiled to `Core/Src/app/sequences.c` by
`tools/gen_sequences.py`. Regenerate after editing:

    python3 tools/gen_sequences.py

Format (everything before the first `# Bank` heading is ignored):

- `# Bank A` / `# Bank B` start a bank (A = LEAD, B = BASSLINE).
- `## name` starts a sequence. Sequence numbers = order in this file.
- A sequence is 4 bars; one line per bar; 8 beats per bar; 4 sub-steps
  per beat (4 PPQN) = 32 tokens per line. `|` separates beats visually
  and is ignored by the parser.
- Tokens: `.` rest · `c2` note (gate + retrigger) · `c2~` tie with pitch
  (legato) · `~` tie continuing the previous pitch · `!` accent suffix.
- `%` repeats the previous bar, `%1`..`%4` repeat bar N of this sequence.
- `>` starts a comment line.
- Octaves: `c0` = 0V; keep leads around octave 2–4, basses 0–2.

The number of sequences per bank is free — firmware, panel and CLI all
derive their ranges from the generated tables.

# Bank A — LEAD

## rising-triad
c3 . . . | e3 . . . | g3 . . . | c4 . . . | g3 . . . | e3 . . . | c3 . . . | . . . .
%
f3 . . . | a3 . . . | c4 . . . | f4 . . . | c4 . . . | a3 . . . | f3 . . . | . . . .
%1

## minor-lift
a2 . . . | c3 . . . | e3 . . . | a3 . . . | e3 . . . | c3 . . . | a2 . . . | . . . .
%
f2 . . . | a2 . . . | c3 . . . | f3 . . . | c3 . . . | a2 . . . | f2 . . . | . . . .
g2 . . . | b2 . . . | d3 . . . | g3 . . . | d3 . . . | b2 . . . | g2 . . . | . . . .

## octave-ping
c3 . c4 . | . c3 . c4 | c3 . c4 . | . . c3 . | c3 . c4 . | . c3 . c4 | c3 . c4 . | . . . .
%
%
eb3 . eb4 . | . eb3 . eb4 | f3 . f4 . | . g3 . g4 | c3 . c4 . | . c3 . c4 | c3 . . . | . . . .

## pent-run
c3 d3 e3 g3 | a3 c4 a3 g3 | e3 g3 e3 d3 | c3 . . . | c3 d3 e3 g3 | a3 c4 d4 c4 | a3 g3 e3 d3 | c3 . . .
%
%
c3 d3 e3 g3 | a3 c4 e4 g4 | e4 c4 a3 g3 | e3 d3 c3 . | g3 . e3 . | d3 . c3 . | d3 e3 d3 c3 | a2 . . .

## acid-16th
c2! c2 c2~ c2 | eb2 . c2 c2 | g2! . c2~ c2 | bb2 . g2 . | c2! c2 c2~ c2 | eb2 . c3 . | c2 . bb2 . | g2 . eb2 .
%
%
c2! c2 c2~ c2 | eb2 . c2 c2 | f2! . f2~ f2 | ab2 . f2 . | g2! g2 g2~ g2 | bb2 . g2 . | c3 . bb2 . | g2 . f2 .

## offbeat-stabs
. . c3 . | . . c3 . | . . eb3 . | . . c3 . | . . f3 . | . . eb3 . | . . c3 . | . . g2 .
%
%
. . c3 . | . . c3 . | . . eb3 . | . . f3 . | . . g3 . | . . bb3 . | . . g3 . | . . f3 .

## call-answer
e3 . g3 a3 | b3 . a3 g3 | e3 . . . | . . . . | e3 . g3 a3 | b3 . d4 b3 | a3 . . . | . . . .
. . . . | e4 . d4 b3 | a3 . . . | . . . . | . . . . | g3 . a3 . | e3 . . . | . . . .
%1
. . . . | e4 . d4 b3 | a3 . b3 . | d4 . b3 . | e4 . . . | . . . . | e3 . . . | . . . .

## arp-updown
c3 e3 g3 c4 | e4 c4 g3 e3 | c3 e3 g3 c4 | e4 c4 g3 e3 | a2 c3 e3 a3 | c4 a3 e3 c3 | f2 a2 c3 f3 | a3 f3 c3 a2
%
%
%

## sus-shimmer
c3 f3 g3 c4 | f3 g3 c4 f4 | g3 c4 f4 g4 | c4 . . . | bb2 eb3 f3 bb3 | eb3 f3 bb3 eb4 | f3 bb3 eb4 f4 | bb3 . . .
%
%
%

## fifth-leaps
c3 . g3 . | d3 . a3 . | e3 . b3 . | g3 . d4 . | c3 . g3 . | d3 . a3 . | c3 g3 c4 g3 | c3 . . .
%
%
f3 . c4 . | g3 . d4 . | a3 . e4 . | g3 . d4 . | f3 . c4 . | e3 . b3 . | d3 a3 d4 a3 | g3 . . .

## trance-pluck
. a2 a2 . | . a2 a2 . | . c3 c3 . | . a2 a2 . | . e3 e3 . | . d3 d3 . | . c3 c3 . | . b2 b2 .
%
%
%

## staircase
c3 . d3 . | e3 . f3 . | g3 . a3 . | b3 . c4 . | c4 . b3 . | a3 . g3 . | f3 . e3 . | d3 . c3 .
%
%
%

## seventh-arp
c3 e3 g3 b3 | c4 b3 g3 e3 | a2 c3 e3 g3 | a3 g3 e3 c3 | f2 a2 c3 e3 | f3 e3 c3 a2 | g2 b2 d3 f3 | g3 f3 d3 b2
%
%
%

## dorian-noodle
d3 . f3 g3 | a3 . g3 f3 | d3 . c3 d3 | . . . . | d3 . f3 g3 | a3 . c4 a3 | b3 . a3 g3 | d3 . . .
%
d3 . f3 g3 | a3 . g3 f3 | d3 . c3 d3 | . . . . | f3 . g3 a3 | c4 . a3 g3 | f3 . d3 c3 | d3 . . .
%1

## stutter-gate
c3 c3 c3 . | c3 c3 . . | c3 . c3 . | c3 c3 c3 c3 | eb3 eb3 eb3 . | eb3 eb3 . . | f3 . f3 . | g3 g3 g3 g3
%
%
c4 c4 c4 . | bb3 bb3 . . | g3 . g3 . | f3 f3 f3 f3 | eb3 eb3 eb3 . | f3 f3 . . | g3 . f3 . | eb3 c3 . .

## long-tones
c3 ~ ~ ~ | ~ ~ ~ ~ | e3 ~ ~ ~ | ~ ~ ~ ~ | g3 ~ ~ ~ | ~ ~ ~ ~ | b3 ~ ~ ~ | c4 ~ ~ ~
%
a3 ~ ~ ~ | ~ ~ ~ ~ | f3 ~ ~ ~ | ~ ~ ~ ~ | g3 ~ ~ ~ | ~ ~ ~ ~ | e3 ~ ~ ~ | d3 ~ ~ ~
c3 ~ ~ ~ | ~ ~ ~ ~ | ~ ~ ~ ~ | ~ ~ ~ ~ | c3 ~ ~ ~ | ~ ~ ~ ~ | ~ ~ ~ ~ | ~ ~ ~ ~

## zigzag
c3 g3 e3 c4 | g3 e4 c4 g3 | a2 e3 c3 a3 | e3 a3 c4 e3 | f2 c3 a2 f3 | c3 f3 a3 c3 | g2 d3 b2 g3 | d3 g3 b3 d3
%
%
%

## chromatic-creep
c3 . . c#3 | . . d3 . | . d#3 . . | e3 . . . | e3 . . f3 | . . f#3 . | . g3 . . | g3 . . .
%
g3 . . f#3 | . . f3 . | . e3 . . | eb3 . . . | d3 . . c#3 | . . c3 . | . c3 . . | c3 . . .
%1

## echo-fade
g3 . g3 . | e3 . . . | g3 . g3 . | e3 . . . | g3 . . . | e3 . . . | g3 . . . | . . . .
g3 . g3 . | e3 . . . | . . g3 . | e3 . . . | . . . . | e3 . . . | . . . . | . . . .
%2
. . . . | e3 . . . | . . . . | . . . . | g3 . . . | . . . . | . . . . | . . . .

## harm-minor-run
a2 b2 c3 d3 | e3 f3 g#3 a3 | a3 g#3 f3 e3 | d3 c3 b2 a2 | a2 c3 e3 a3 | e3 c3 a2 . | g#2 b2 e3 g#3 | e3 b2 g#2 .
%
%
%

## wide-interval
c3 . . . | c4 . . . | e3 . . . | e4 . . . | g3 . . . | g4 . . . | e4 . c4 . | g3 . . .
%
a2 . . . | a3 . . . | c3 . . . | c4 . . . | f3 . . . | f4 . . . | c4 . a3 . | f3 . . .
%1

## maj7-broken
c3 . e3 g3 | b3 . g3 e3 | c3 . e3 g3 | b3 . . . | f3 . a3 c4 | e4 . c4 a3 | f3 . a3 c4 | e4 . . .
%
%
%

## rave-riff
a3 a3 . a3 | . g3 a3 . | a3 a3 . a3 | . c4 a3 . | a3 a3 . a3 | . g3 a3 . | e3 . g3 . | a3 . . .
%
%
a3 a3 . a3 | . g3 a3 . | c4 c4 . c4 | . d4 c4 . | e4 . d4 . | c4 . a3 . | g3 . e3 . | a3 . . .

## three-against-four
c3 . . e3 | . . g3 . | . c4 . . | e4 . . g3 | . . c3 . | . e3 . . | g3 . . c4 | . . . .
%
%
%

## two-oct-arp
c3 e3 g3 c4 | e4 g4 e4 c4 | g3 e3 c3 e3 | g3 c4 g3 e3 | a2 c3 e3 a3 | c4 e4 c4 a3 | e3 c3 a2 c3 | e3 a3 e3 c3
%
%
%

## pedal-return
c4 . a3 . | c4 . g3 . | c4 . f3 . | c4 . e3 . | c4 . d3 . | c4 . e3 . | c4 . f3 . | c4 . g3 .
%
%
%

## quartal
c3 . f3 . | bb3 . eb4 . | . . bb3 . | f3 . . . | d3 . g3 . | c4 . f4 . | . . c4 . | g3 . . .
%
%
e3 . a3 . | d4 . g4 . | . . d4 . | a3 . . . | c3 . f3 . | bb3 . eb4 . | bb3 . f3 . | c3 . . .

## bell-tones
e4 ~ ~ ~ | ~ ~ . . | b3 ~ ~ ~ | ~ ~ . . | g4 ~ ~ ~ | ~ ~ . . | f#4 ~ ~ ~ | ~ ~ . .
%
d4 ~ ~ ~ | ~ ~ . . | a3 ~ ~ ~ | ~ ~ . . | e4 ~ ~ ~ | ~ ~ . . | b3 ~ ~ ~ | ~ ~ . .
%1

## e-min-ostinato
e3 g3 b3 e4 | g3 b3 e3 g3 | e3 g3 b3 e4 | b3 g3 e3 . | c3 e3 g3 c4 | e3 g3 c3 e3 | d3 f#3 a3 d4 | a3 f#3 d3 .
%
%
%

## blues-lick
c3 . eb3 f3 | f#3 . g3 . | bb3 . g3 . | f3 eb3 c3 . | c3 . eb3 f3 | f#3 g3 . bb3 | c4 . bb3 g3 | f3 . eb3 c3
%
%
%

## syncopa
c3 . . c3 | . c3 . . | eb3 . . eb3 | . eb3 . . | f3 . . f3 | . g3 . . | bb3 . . g3 | . f3 . eb3
%
%
c4 . . bb3 | . g3 . . | f3 . . eb3 | . c3 . . | c3 . . c3 | . eb3 . . | f3 . g3 . | c3 . . .

## cascade
c4 b3 a3 g3 | f3 e3 d3 c3 | c4 . g3 . | e3 . c3 . | b3 a3 g3 f3 | e3 d3 c3 b2 | c3 ~ ~ ~ | ~ ~ ~ ~
%
%
e4 d4 c4 b3 | a3 g3 f3 e3 | e4 . b3 . | g3 . e3 . | d4 c4 b3 a3 | g3 f3 e3 d3 | c3 ~ ~ ~ | ~ ~ ~ ~

# Bank B — BASSLINE

## root-eighths
c1 . c1 . | c1 . c1 . | c1 . c1 . | c1 . c1 . | c1 . c1 . | c1 . c1 . | c1 . c1 . | c1 . c1 .
%
%
c1 . c1 . | c1 . c1 . | bb0 . bb0 . | bb0 . bb0 . | g0 . g0 . | g0 . g0 . | bb0 . bb0 . | b0 . b0 .

## root-fifth
c1 . . . | g1 . . . | c1 . . . | g1 . . . | c1 . . . | g1 . . . | c1 . g1 . | c1 . . .
%
f1 . . . | c2 . . . | f1 . . . | c2 . . . | g1 . . . | d2 . . . | g1 . d2 . | g1 . . .
%1

## octave-pump
c1 . c2 . | c1 . c2 . | c1 . c2 . | c1 . c2 . | eb1 . eb2 . | eb1 . eb2 . | f1 . f2 . | g1 . g2 .
%
%
%

## offbeat-sub
. . c1 . | . . c1 . | . . c1 . | . . c1 . | . . eb1 . | . . eb1 . | . . f1 . | . . g1 .
%
%
%

## acid-bass
c1! c1 c1~ c1 | c2 . c1 c1 | eb1 . c1~ c1 | g1 . eb1 . | c1! c1 c1~ c1 | bb1 . c1 c1 | g1 . eb1 . | c1 . . .
%
%
c1! c1 c1~ c1 | c2 . c1 c1 | f1! . f1~ f1 | ab1 . f1 . | g1! . g1~ g1 | bb1 . g1 . | c2 . bb1 . | g1 . f1 .

## walking
c1 . e1 . | g1 . a1 . | c2 . a1 . | g1 . e1 . | f1 . a1 . | c2 . a1 . | g1 . f1 . | e1 . d1 .
%
%
c1 . e1 . | g1 . a1 . | bb1 . a1 . | g1 . f1 . | e1 . g1 . | c1 . e1 . | g0 . b0 . | c1 . . .

## sub-pedal
c1 ~ ~ ~ | ~ ~ ~ ~ | ~ ~ ~ ~ | ~ ~ ~ ~ | c1 ~ ~ ~ | ~ ~ ~ ~ | ~ ~ ~ ~ | ~ ~ ~ ~
%
ab0 ~ ~ ~ | ~ ~ ~ ~ | ~ ~ ~ ~ | ~ ~ ~ ~ | bb0 ~ ~ ~ | ~ ~ ~ ~ | ~ ~ ~ ~ | ~ ~ ~ ~
%1

## syncop-funk
c1 . . c1 | . c1 . . | c1 . . c1 | . . c1 . | c1 . . c1 | . c1 . . | eb1 . . f1 | . . g1 .
%
%
c1 . . c1 | . c1 . . | f1 . . f1 | . . f1 . | g1 . . g1 | . g1 . . | bb1 . . g1 | . . c1 .

## minor-groove
a0 . a0 . | a1 . a0 . | a0 . a0 . | g1 . e1 . | a0 . a0 . | a1 . a0 . | c1 . d1 . | e1 . g1 .
%
%
%

## fifth-oct
c1 . g1 . | c2 . g1 . | c1 . g1 . | c2 . g1 . | f1 . c2 . | f2 . c2 . | g1 . d2 . | g1 . . .
%
%
%

## rolling-16ths
c1 c1 c1 c1 | c1 c1 c1 c1 | c1 c1 c1 c1 | c1 c1 c1 c1 | eb1 eb1 eb1 eb1 | eb1 eb1 eb1 eb1 | f1 f1 f1 f1 | g1 g1 g1 g1
%
%
c1! c1 c1 c1 | c1 c1 c1 c1 | bb0! bb0 bb0 bb0 | bb0 bb0 bb0 bb0 | ab0! ab0 ab0 ab0 | ab0 ab0 ab0 ab0 | g0! g0 g0 g0 | g0 g0 g0 g0

## dub-sparse
c1 . . . | . . . . | . . c1 . | . . . . | c1 . . . | . . . . | . . eb1 . | . . . .
%
%
c1 . . . | . . . . | . . c1 . | . . . . | g0 . . . | . . . . | . . bb0 . | . . . .

## climb
c1 . d1 . | eb1 . f1 . | g1 . . . | . . . . | c1 . d1 . | eb1 . f1 . | g1 . ab1 . | bb1 . . .
%
%
c2 . bb1 . | ab1 . g1 . | f1 . eb1 . | d1 . c1 . | c1 . d1 . | eb1 . f1 . | g1 . f1 . | eb1 . d1 .

## drop-octave
c2 . . . | c1 . . . | c2 . . . | c1 . . . | eb2 . . . | eb1 . . . | f2 . . . | f1 . . .
%
%
g2 . . . | g1 . . . | f2 . . . | f1 . . . | eb2 . . . | eb1 . . . | c2 . . . | c1 . . .

## dark-halfstep
a0 . . . | bb0 . . . | a0 . . . | bb0 . . . | a0 . a0 . | bb0 . a0 . | e1 . . . | f1 . . .
%
%
%

## seventh-walk
c1 . e1 . | g1 . bb1 . | c2 . bb1 . | g1 . e1 . | c1 . e1 . | g1 . bb1 . | a1 . f1 . | g1 . b0 .
%
%
%

## tritone-tension
c1 . . . | f#1 . . . | c1 . . . | f#1 . . . | c1 . c1 . | f#1 . f#1 . | g1 . . . | f1 . . .
%
%
%

## legato-slide
c1 ~ ~ ~ | d1~ ~ ~ ~ | eb1~ ~ ~ ~ | ~ ~ ~ ~ | g1~ ~ ~ ~ | f1~ ~ ~ ~ | eb1~ ~ ~ ~ | c1~ ~ ~ ~
%
%
c1 ~ ~ ~ | d1~ ~ ~ ~ | eb1~ ~ ~ ~ | f1~ ~ ~ ~ | g1~ ~ ~ ~ | ab1~ ~ ~ ~ | bb1~ ~ ~ ~ | c2~ ~ ~ ~

## bounce-thirds
c1 . eb1 . | c1 . eb1 . | c1 . eb1 . | c1 eb1 c1 . | f1 . ab1 . | f1 . ab1 . | g1 . bb1 . | g1 bb1 g1 .
%
%
%

## turnaround
c1 . . . | c1 . g0 . | c1 . . . | c1 . g0 . | c1 . . . | c1 . g0 . | c1 . . . | c1 . g0 .
%
%
c1 . . . | c1 . . . | f1 . . . | f1 . . . | g1 . a1 . | b1 . . . | c2 . . . | g1 . . .

## gallop
c1 . c1 c1 | c1 . c1 c1 | c1 . c1 c1 | c1 . c1 c1 | eb1 . eb1 eb1 | eb1 . eb1 eb1 | f1 . f1 f1 | g1 . g1 g1
%
%
%

## halfbar-answer
c1 . c1 . | c1 c1 . . | . . . . | . . . . | f1 . f1 . | f1 f1 . . | . . . . | . . . .
%
%
c1 . c1 . | c1 c1 . . | eb1 . eb1 . | eb1 eb1 . . | f1 . f1 . | f1 f1 . . | g1 . g1 . | bb1 bb1 . .

## deep-drone
c1 ~ ~ ~ | ~ ~ ~ ~ | ~ ~ ~ ~ | ~ ~ ~ ~ | ~ ~ ~ ~ | ~ ~ ~ ~ | ~ ~ ~ ~ | ~ ~ ~ ~
%
%
bb0 ~ ~ ~ | ~ ~ ~ ~ | ~ ~ ~ ~ | ~ ~ ~ ~ | g0 ~ ~ ~ | ~ ~ ~ ~ | ~ ~ ~ ~ | ~ ~ ~ ~

## offbeat-oct
. c2 . c1 | . c2 . c1 | . c2 . c1 | . c2 . c1 | . eb2 . eb1 | . eb2 . eb1 | . f2 . f1 | . g2 . g1
%
%
%

## rise-fall
c1 . e1 . | g1 . e1 . | c1 . e1 . | g1 . e1 . | a0 . c1 . | e1 . c1 . | f0 . a0 . | c1 . a0 .
%
%
%

## stab-rest
c1 . . . | . . . . | . . . . | c1 . . . | . . . . | . . . . | eb1 . . . | . . . .
%
f1 . . . | . . . . | . . . . | f1 . . . | . . . . | . . . . | g1 . . . | . . . .
%1

## minor-arp
a0 c1 e1 a1 | e1 c1 a0 c1 | a0 c1 e1 a1 | e1 c1 a0 . | f0 a0 c1 f1 | c1 a0 f0 a0 | g0 b0 d1 g1 | d1 b0 g0 .
%
%
%

## driving-accents
c1! c1 c1 c1 | c1! c1 c1 c1 | c1! c1 c1 c1 | c1! c1 g1 g1 | c1! c1 c1 c1 | c1! c1 c1 c1 | eb1! eb1 eb1 eb1 | f1! f1 g1 g1
%
%
%

## broken-oct-16th
c1 c2 c1 c2 | c1 c2 c1 c2 | c1 c2 c1 c2 | c1 c2 c1 c2 | ab0 ab1 ab0 ab1 | ab0 ab1 ab0 ab1 | bb0 bb1 bb0 bb1 | bb0 bb1 bb0 bb1
%
%
%

## approach-pedal
c1 . . . | c1 . . . | c1 . . . | b0 . . . | c1 . . . | c1 . . . | c1 . . . | d1 . . .
%
%
c1 . . . | c1 . . . | f1 . . . | e1 . . . | g1 . . . | g1 . . . | b0 . . . | c1 . . .

## wide-sub-jump
c1 . . . | g0 . . . | c1 . . . | g0 . . . | c1 . c1 . | g0 . . . | f0 . . . | g0 . . .
%
%
%

## finale-walkdown
c2 . b1 . | a1 . g1 . | f1 . e1 . | d1 . c1 . | c2 . b1 . | a1 . g1 . | f1 . g1 . | c1 . . .
%
%
c2 . b1 . | a1 . g1 . | f1 . e1 . | d1 . c1 . | c1 ~ ~ ~ | ~ ~ ~ ~ | ~ ~ ~ ~ | ~ ~ ~ ~
