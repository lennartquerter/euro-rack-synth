const panelWidth = 40;
const panelHeight = 128.5;

const pcbWidth = 40;
const pcbHeight = 111;

const topOffset = (panelHeight - pcbHeight) / 2;
const rightOffset = (panelWidth - pcbWidth) / 2;

const holePositions = [
    {
        label: 'power_led',
        name: 'power_led',
        fromTop: 11,
        fromRight: 36.07,
        diameter: 7.2
    },
    {
        label: 'rate_led',
        name: 'rate_led',
        fromTop: 6,
        fromRight: 20,
        diameter: 7.2
    },
    {
        label: 'rate',
        name: 'rate',
        fromTop: 22,
        fromRight: 20,
        diameter: 7.2
    },
    {
        label: 'sq_range',
        name: 'sq_range',
        fromTop: 50,
        fromRight: 31,
        diameter: 7.2
    },
    {
        label: 'tri_range',
        name: 'tri_range',
        fromTop: 50,
        fromRight: 9,
        diameter: 7.2
    },
    {
        label: 'sw_rate',
        name: 'sw_rate',
        fromTop: 75,
        fromRight: 20,
        diameter: 7.2
    },
    {
        label: 'sin_out',
        name: 'sin_out',
        fromTop: 90,
        fromRight: 20,
        diameter: 6.2
    },
    {
        label: 'tri_out',
        name: 'tri_out',
        fromTop: 95,
        fromRight: 6,
        diameter: 6.2
    },
    {
        label: 'sq_out',
        name: 'sq_out',
        fromTop: 95,
        fromRight: 34,
        diameter: 6.2
    },

]

class PanelConverter {

    convert() {
        holePositions.forEach((hole) => {
                const fromTop = topOffset + hole.fromTop;
                const fromRight = rightOffset + hole.fromRight;
                console.log(`Hole ${hole.label} \t top: ${fromTop}\t  right: ${fromRight}\t  diameter: ${hole.diameter}`);
            }
        );
    }
}

const panelConverter = new PanelConverter();
panelConverter.convert();