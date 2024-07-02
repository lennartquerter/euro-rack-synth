const panelWidth = 42;
const panelHeight = 128.5;

const pcbWidth = 42;
const pcbHeight = 111;

const topOffset = (panelHeight - pcbHeight) / 2;
const rightOffset = (panelWidth - pcbWidth) / 2;

const holePositions = [
    {
        label: '1_input',
        name: '1_input',
        fromTop: 22.12,
        fromRight: 31.1,
        diameter: 6.2
    },
    {
        label: '1_cv',
        name: '1_cv',
        fromTop: 22.12,
        fromRight: 20,
        diameter: 6.2
    },
    {
        label: '1_out',
        name: '1_out',
        fromTop: 22.12,
        fromRight: 8.9,
        diameter: 6.2
    },
    {
        label: '1_ctrl',
        name: '1_ctrl',
        fromTop: 46.075,
        fromRight: 20,
        diameter: 7.2
    },
    {
        label: '2_input',
        name: '2_input',
        fromTop: 97.98,
        fromRight: 31.1,
        diameter: 6.2
    },
    {
        label: '2_cv',
        name: '2_cv',
        fromTop: 97.98,
        fromRight: 20,
        diameter: 6.2
    },
    {
        label: '2_out',
        name: '2_out',
        fromTop: 97.98,
        fromRight: 8.9,
        diameter: 6.2
    },
    {
        label: '2_ctrl',
        name: '2_ctrl',
        fromTop: 74,
        fromRight: 20,
        diameter: 7.2
    },
]

class PanelConverter {
    convert() {
        holePositions.forEach((hole) => {
                const fromTop = topOffset + hole.fromTop;
                const fromRight = rightOffset + hole.fromRight;
                console.log(`Hole ${hole.label} \t\t top: ${fromTop}\t  right: ${fromRight}\t  diameter: ${hole.diameter}`);
            }
        );
    }
}

const panelConverter = new PanelConverter();
panelConverter.convert();