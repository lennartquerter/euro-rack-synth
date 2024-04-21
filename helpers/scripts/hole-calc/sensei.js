const panelWidth = 72;
const panelHeight = 129.5;

const pcbWidth = 70;
const pcbScoreOffset = 2;
const pcbHeight = 111;

const topOffset = (panelHeight - pcbHeight) / 2;
const rightOffset = (panelWidth - pcbWidth) / 2;
const bottomOffset = (panelHeight - pcbHeight) / 2;
const leftOffset = (panelWidth - pcbWidth) / 2;

const holePositions = [
    {
        label: '1_CV',
        name: '1_CV',
        fromTop: 13.5,
        fromRight: 57.5,
        diameter: 7.2
    },
    {
        label: '1_SW',
        name: '1_SW',
        fromTop: 29.5,
        fromRight: 57.5,
        diameter: 6.2
    },
    {
        label: '1_LED',
        name: '1_LED',
        fromTop: 36,
        fromRight: 57.5,
        diameter: 3.1
    },
    {
        label: '2_CV',
        name: '2_CV',
        fromTop: 13.5,
        fromRight: 42.5,
        diameter: 7.2
    },
    {
        label: '2_SW',
        name: '2_SW',
        fromTop: 29.5,
        fromRight: 42.5,
        diameter: 6.2
    },
    {
        label: '2_LED',
        name: '2_LED',
        fromTop: 36,
        fromRight: 42.5,
        diameter: 3.1
    },
    {
        label: '3_CV',
        name: '3_CV',
        fromTop: 13.5,
        fromRight: 27.5,
        diameter: 7.2
    },
    {
        label: '3_SW',
        name: '3_SW',
        fromTop: 29.5,
        fromRight: 27.5,
        diameter: 6.2
    },
    {
        label: '3_LED',
        name: '3_LED',
        fromTop: 36,
        fromRight: 27.5,
        diameter: 3.1
    },
    {
        label: '4_CV',
        name: '4_CV',
        fromTop: 13.5,
        fromRight: 12.8,
        diameter: 7.2
    },
    {
        label: '4_SW',
        name: '4_SW',
        fromTop: 29.5,
        fromRight: 12.5,
        diameter: 6.2
    },
    {
        label: '4_LED',
        name: '4_LED',
        fromTop: 36,
        fromRight: 12.5,
        diameter: 3.1
    },
    {
        label: '5_CV',
        name: '5_CV',
        fromTop: 53.5,
        fromRight: 57.5,
        diameter: 7.2
    },
    {
        label: '5_SW',
        name: '5_SW',
        fromTop: 69.5,
        fromRight: 57.5,
        diameter: 6.2
    },
    {
        label: '5_LED',
        name: '5_LED',
        fromTop: 76,
        fromRight: 57.5,
        diameter: 3.1
    },
    {
        label: '6_CV',
        name: '6_CV',
        fromTop: 53.5,
        fromRight: 42.5,
        diameter: 7.2
    },
    {
        label: '6_SW',
        name: '6_SW',
        fromTop: 69.5,
        fromRight: 42.5,
        diameter: 6.2
    },
    {
        label: '6_LED',
        name: '6_LED',
        fromTop: 76,
        fromRight: 42.5,
        diameter: 3.1
    },
    {
        label: '7_CV',
        name: '7_CV',
        fromTop: 53.5,
        fromRight: 27.5,
        diameter: 7.2
    },
    {
        label: '7_SW',
        name: '7_SW',
        fromTop: 69.5,
        fromRight: 27.5,
        diameter: 6.2
    },
    {
        label: '7_LED',
        name: '7_LED',
        fromTop: 76,
        fromRight: 27.5,
        diameter: 3.1
    },
    {
        label: '8_CV',
        name: '8_CV',
        fromTop: 53.5,
        fromRight: 12.5,
        diameter: 7.2
    },
    {
        label: '8_SW',
        name: '8_SW',
        fromTop: 69.5,
        fromRight: 12.5,
        diameter: 6.2
    },
    {
        label: '8_LED',
        name: '8_LED',
        fromTop: 76,
        fromRight: 12.5,
        diameter: 3.1
    },
    {
        label: 'CLK_SPD',
        name: 'CLK_SPD',
        fromTop: 89.5,
        fromRight: 57.5,
        diameter: 7.2
    },
    {
        label: 'CLK_INPUT',
        name: 'CLK_INPUT',
        fromTop: 89.5,
        fromRight: 43.98,
        diameter: 6.2
    },
    {
        label: 'CLK_OUTPUT',
        name: 'CLK_OUTPUT',
        fromTop: 99.75,
        fromRight: 43.98,
        diameter: 6.2
    },
    {
        label: 'CV_OUT',
        name: 'CV_OUT',
        fromTop: 89.5,
        fromRight: 25.77,
        diameter: 6.2
    },
    {
        label: 'GATE_OUT',
        name: 'GATE_OUT',
        fromTop: 99.75,
        fromRight: 25.77,
        diameter: 6.2
    },
    {
        label: 'CLK_SPD_SW',
        name: 'CLK_SPD_SW',
        fromTop: 89.5,
        fromRight: 8.8,
        diameter: 6.2
    },
    {
        label: 'CV_LVL',
        name: 'CV_LVL',
        fromTop: 98,
        fromRight: 8.8,
        diameter: 6.2
    },
    {
        label: 'TRIM',
        name: 'TRIM',
        fromTop: 104.7,
        fromRight: 56.25,
        diameter: 6.2
    },
    {
        label: 'SW_Gutter_01',
        name: 'TRIM',
        fromTop: 25.24,
        fromRight: 0,
        diameter: 0
    },
    {
        label: 'SW_Gutter_01',
        name: 'TRIM',
        fromTop: 65.22,
        fromRight: 0,
        diameter: 0
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