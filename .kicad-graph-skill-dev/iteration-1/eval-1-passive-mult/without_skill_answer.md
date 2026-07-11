# Passive Multiple Jack Configuration

## Summary
- **Input jacks:** 2 (J1, J2)
- **Output jacks:** 6 (J3, J4, J5, J6, J7, J8)

## Detailed Wiring

### Input Jacks
- **J1** at (109.22, 58.42) — First input
- **J2** at (109.22, 91.44) — Second input

### Output Jacks (all at x=146.05)
- **J3** at y=58.42 — OUTPUT
- **J4** at y=68.58 — OUTPUT
- **J5** at y=78.74 — OUTPUT
- **J6** at y=91.44 — OUTPUT
- **J7** at y=101.6 — OUTPUT
- **J8** at y=111.76 — OUTPUT

## Wiring Topology & Normalling Behavior

The schematic uses **switched audio jacks** (each has pins S, T, and TN) where:
- **S** = Sleeve (ground)
- **T** = Tip (signal)
- **TN** = Tip Normalling (contacts ground when jack is unplugged)

### Signal Path (Passive Mult Topology)

**Input J1 → Outputs J3 & J4** (tied together):
- J1.T (pin T at 109.22, 58.42) → junction at (125.73, 58.42)
- This signal flows vertically to junction (125.73, 68.58)
- (125.73, 58.42) connects → J3.T at (146.05, 58.42) [wire uuid fb1bfa9c-0a99...]
- (125.73, 68.58) connects → J4.T at (146.05, 68.58) [wire uuid 0529a705-0293...]

**Input J2 → Outputs J5, J6, J7, J8** (all tied together):
- J2.T at (109.22, 91.44) → junction at (125.73, 91.44) [wire uuid 4a305415...]
- J2 also has **normalling behavior** through TN contact:
  - J2.TN → (120.65, 78.74) [wire uuid 4cf8842c...]
  - (120.65, 78.74) → (125.73, 78.74) [wire uuid c2e0ea1d...]
  - This creates a normalling chain through J5

The four outputs (J5–J8) share a common signal rail at y-coordinates that form a vertical bus:
- (125.73, 78.74) ↔ (125.73, 68.58) [vertical, uuid 63bbb2e7...]
- (125.73, 101.6) ↔ (125.73, 91.44) [vertical, uuid 94742bfe...]
- (125.73, 101.6) → J7.T at (146.05, 101.6) [wire uuid a1bc7209...]
- (125.73, 91.44) → J6.T at (146.05, 91.44) [wire uuid c3cd2ff5...]
- (125.73, 111.76) ↔ (125.73, 101.6) [vertical, uuid 20915c5c...]
- (140.97, 111.76) → J8.T [wire uuid 1d122120...]

Additional normalling on J2:
- J2.TN feeds into (120.65, 93.98) [wire uuid 6c4b5ca7...]

## Summary of Tied Outputs
1. **J3 & J4 are tied together** — both receive the signal from J1
2. **J5, J6, J7, J8 are tied together** — all receive the signal from J2
3. **Normalling contacts** on both input jacks (J1.TN and J2.TN) are present but marked with `no_connect` in the schematic, indicating they are not actively used for jack normalling in this design

## Component References
- Input jacks: J1, J2
- Output jacks: J3, J4, J5, J6, J7, J8
- All jacks use `AudioJack_Mono_3.5mm` symbol with switched tip contact
- Ground symbols (GNDREF) connect to all jack sleeve pins via connections to x=140.97 at appropriate y-coordinates
