import {ModulesRepository} from "../db/modules.repository";
import {EuroRackModuleEntity} from "../models/euroRackModule.entity";
import {CONSTANTS} from "../const/constants";

export class PcbCalculatorService {

    constructor(private readonly repo: ModulesRepository) {
    }

    public calculate(key: string) {
        const module: EuroRackModuleEntity = this.repo.getModuleWithName(key);

        const topOffset = (module.frontPanel.heightInMM - module.pcb.heightInMM) / 2;
        const rightOffset = (module.frontPanel.widthInMM - module.pcb.widthInMM) / 2;

        module.holes.forEach(hole => {
            const fromTop = topOffset + hole.distanceFromTopInMM;
            const fromRight = rightOffset + hole.distanceFromRightInMM;
            const convertedDiameter = CONSTANTS.holeToDimension[hole.type]

            console.log(`Hole ${hole.label} \t top: ${fromTop}\t  right: ${fromRight}\t  diameter: ${convertedDiameter}`);
        })
    }
}