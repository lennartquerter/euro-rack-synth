# 3320 Filter Power Rail Sanity Check

## Power Rail Pin Counts

### +12V Rail
- **Total pins: 24** (excluding power flag pseudo-components)
- Actual components: 3 op-amps + bypass caps + other parts
- Op-amp connections: **U1 pin 8**, **U2 pin 4**, **U3 pin 14**

### -12V Rail
- **Total pins: 25** (excluding power flag pseudo-components)
- Actual components: 2 op-amps + bypass caps + other parts
- Op-amp connections: **U1 pin 4**, **U2 pin 11**

### GNDREF Rail
- **Total pins: 94** (excluding power flag pseudo-components)
- Extensive ground distribution across all components
- Op-amp connections: **U1 pins 3 & 5**, **U2 pins 5 & 10**, **U3 pin 3**

---

## Op-Amp Supply Connection Summary

### U1: TL072 (Dual JFET Op-Amp)
✓ **Properly supplied**
- Pin 8 (V+): Connected to **+12V**
- Pin 4 (V-): Connected to **-12V**
- Pin 3 (GND): Connected to **GNDREF**
- Pin 5 (GND): Connected to **GNDREF**

### U2: TL074 (Quad JFET Op-Amp)
✓ **Properly supplied**
- Pin 4 (V+): Connected to **+12V**
- Pin 11 (V-): Connected to **-12V**
- Pin 5 (GND): Connected to **GNDREF**
- Pin 10 (GND): Connected to **GNDREF**

### U3: CEM3320 (Integrated Filter Chip)
✓ **Properly supplied**
- Pin 14 (VCC): Connected to **+12V**
- Pin 3 (GND): Connected to **GNDREF**
- **Note:** CEM3320 is single-supply only—no negative supply pin required

---

## Bypass Capacitor Analysis

All bypass capacitors are present and correctly connected:

### +12V Rail Bypass Caps (5 total)
- **C1**: +12V → GNDREF ✓
- **C3**: +12V → GNDREF ✓
- **C5**: +12V → GNDREF ✓
- **C7**: +12V → GNDREF ✓
- **C15**: +12V → GNDREF ✓

### -12V Rail Bypass Caps (4 total)
- **C2**: -12V → GNDREF ✓
- **C4**: -12V → GNDREF ✓
- **C6**: -12V → GNDREF ✓
- **C8**: -12V → GNDREF ✓

---

## Conclusion

**All power rails are properly configured:**
- ✓ All three chips (U1, U2, U3) have complete and correct supply connections
- ✓ All supply pins are on the proper rails (+12V, -12V, GNDREF)
- ✓ No missing supply connections detected
- ✓ Comprehensive bypass capacitor coverage on both +12V and -12V rails
- ✓ No floating or unconnected supply pins

**No design issues identified.**
