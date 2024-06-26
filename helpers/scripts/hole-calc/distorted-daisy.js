const panelWidth = 32;
const panelHeight = 129.5;

const pcbWidth = 32;
const pcbScoreOffset = 2;
const pcbHeight = 111;

const topOffset = (panelHeight - pcbHeight) / 2;
const rightOffset = (panelWidth - pcbWidth) / 2;
const bottomOffset = (panelHeight - pcbHeight) / 2;
const leftOffset = (panelWidth - pcbWidth) / 2;

const holePositions = [
    {
        label: '1_input',
        name: '1_input',
        fromTop: 12.9,
        fromRight: 21.42,
        diameter: 6.1
    },
    {
        label: '1_led',
        name: '1_led',
        fromTop: 4.9,
        fromRight: 21.5,
        diameter: 3.0
    },
    {
        label: '1_ctrl',
        name: '1_ctrl',
        fromTop: 12.9,
        fromRight: 7.25,
        diameter: 6.1
    },
    {
        label: '2_input',
        name: '2_input',
        fromTop: 28.375,
        fromRight: 21.42,
        diameter: 6.1
    },
    {
        label: '2_led',
        name: '2_led',
        fromTop: 20.425,
        fromRight: 21.5,
        diameter: 3.0
    },
    {
        label: '2_ctrl',
        name: '2_ctrl',
        fromTop: 28.375,
        fromRight: 7.25,
        diameter: 6.1
    },
    {
        label: '3_input',
        name: '3_input',
        fromTop: 43.9,
        fromRight: 21.42,
        diameter: 6.1
    },
    {
        label: '3_led',
        name: '3_led',
        fromTop: 35.9,
        fromRight: 21.5,
        diameter: 3.0
    },
    {
        label: '3_ctrl',
        name: '3_ctrl',
        fromTop: 43.9,
        fromRight: 7.25,
        diameter: 6.1
    },
    {
        label: '4_input',
        name: '4_input',
        fromTop: 59.4,
        fromRight: 21.42,
        diameter: 6.1
    },
    {
        label: '4_led',
        name: '4_led',
        fromTop: 51.4,
        fromRight: 21.5,
        diameter: 3.0
    },
    {
        label: '4_ctrl',
        name: '4_ctrl',
        fromTop: 59.4,
        fromRight: 7.25,
        diameter: 6.1
    },
    {
        label: 'distorted_ctrl',
        name: 'distorted_ctrl',
        fromTop: 76.075,
        fromRight: 21.4,
        diameter: 7.2
    },
    {
        label: 'distorted_out',
        name: 'distorted_out',
        fromTop: 76.02,
        fromRight: 7.2,
        diameter: 6.1
    },
    {
        label: 'ctrl',
        name: 'ctrl',
        fromTop: 98,
        fromRight: 23,
        diameter: 7.2
    },
    {
        label: 'out',
        name: 'out',
        fromTop: 98,
        fromRight: 7.8,
        diameter: 6.1
    }, {
        label: 'out_led',
        name: 'out_led',
        fromTop: 90.5,
        fromRight: 7.8,
        diameter: 3.0
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