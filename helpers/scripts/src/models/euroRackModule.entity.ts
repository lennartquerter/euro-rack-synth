import {EuroRackHoleEntity} from "./EuroRackHole.entity";

export interface EuroRackModuleEntity {
    holes: EuroRackHoleEntity[]
    frontPanel: {
        heightInMM: number
        widthInMM: number
    },
    pcb: {
        heightInMM: number
        widthInMM: number
    }
}