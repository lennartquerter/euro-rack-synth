import * as modules from './modules.json';


import {EuroRackModuleEntity} from "../models/euroRackModule.entity";

export class ModulesRepository {

    public getModuleWithName(key: string): EuroRackModuleEntity {
        return (modules as Record<string, EuroRackModuleEntity>)[key]
    }
}