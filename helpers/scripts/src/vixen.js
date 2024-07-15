const panelWidth = 40.8;
const panelHeight = 128.5;

const pcbWidth = 40.3;
const pcbHeight = 111;

const topOffset = (panelHeight - pcbHeight) / 2;
const rightOffset = (panelWidth - pcbWidth) / 2;

const holePositions = [
    {
        label: 'course1',
        name: 'course1',
        fromTop: 6.5,
        fromRight: 30.3,
        diameter: 7.2
    },
    {
        label: 'fine1',
        name: 'fine1',
        fromTop: 22.5,
        fromRight: 9.95,
        diameter: 7.2
    },
    {
        label: 'pwm_cntr1',
        name: 'pwm_cntr1',
        fromTop: 47.5,
        fromRight: 30.12,
        diameter: 7.2
    },
    {
        label: 'pwm_amt1',
        name: 'pwm_amt1',
        fromTop: 47.5,
        fromRight: 9.95,
        diameter: 7.2
    },
    {
        label: 'fm',
        name: 'fm',
        fromTop: 62.5,
        fromRight: 20.05,
        diameter: 7.2
    },
    {
        label: '1v/oct',
        name: '1v/oct',
        fromTop: 88.58,
        fromRight: 31.55,
        diameter: 6.2
    },
    {
        label: 'fm_in',
        name: 'fm_in',
        fromTop: 88.58,
        fromRight: 20.05,
        diameter: 6.2
    },
    {
        label: 'pwm_in',
        name: 'pwm_in',
        fromTop: 88.58,
        fromRight: 8.8,
        diameter: 6.2
    },
    {
        label: 'pulse_out',
        name: 'pulse_out',
        fromTop: 103.5,
        fromRight: 31.55,
        diameter: 6.2
    },
    {
        label: 'saw_out',
        name: 'saw_out',
        fromTop: 103.5,
        fromRight: 8.8,
        diameter: 6.2
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