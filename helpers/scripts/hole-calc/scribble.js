const panelWidth = 43;
const panelHeight = 129.5;

const pcbWidth = 42;
const pcbScoreOffset = 2;
const pcbHeight = 110;

const topOffset = (panelHeight - pcbHeight) / 2;
const rightOffset = (panelWidth - pcbWidth) / 2;
const bottomOffset = (panelHeight - pcbHeight) / 2;
const leftOffset = (panelWidth - pcbWidth) / 2;

const holePositions = [
    {
        label: 'GATE_INPUT',
        name: 'GATE_INPUT',
        fromTop: 12,
        fromRight: 30.5,
        diameter: 6.2
    },
    {
        label: 'LED',
        name: 'LED',
        fromTop: 6.5,
        fromRight: 20.5,
        diameter: 5
    },
    {
        label: 'ATTACK',
        name: 'ATTACK',
        fromTop: 38.25,
        fromRight: 20.5,
        diameter: 7.2
    },
    {
        label: 'SUS/REL',
        name: 'SUS/REL',
        fromTop: 54.9750,
        fromRight: 20.5,
        diameter: 7.2
    },
    {
        label: 'SUS',
        name: 'SUS',
        fromTop: 71.75,
        fromRight: 20.5,
        diameter: 7.2
    },

    {
        label: 'INV_OUT',
        name: 'INV_OUT',
        fromTop: 100.08,
        fromRight: 30.5,
        diameter: 6.2
    },

    {
        label: 'OUT',
        name: 'OUT',
        fromTop: 100.08,
        fromRight: 10.5019,
        diameter: 6.2
    },
]

class PanelConverter {

    convert() {
        holePositions.forEach((hole) => {
                const fromTop = topOffset + hole.fromTop;
                const fromRight = rightOffset + hole.fromRight;
                console.log(`Hole ${hole.label} at top: ${fromTop}, right: ${fromRight}, diameter: ${hole.diameter}`);
            }
        );
    }
}

const panelConverter = new PanelConverter();
panelConverter.convert();