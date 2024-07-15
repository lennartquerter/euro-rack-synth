import {PcbCalculatorService} from "./services/pcbCalculator.service";
import {ModulesRepository} from "./db/modules.repository";

const repo = new ModulesRepository()

const pcbService = new PcbCalculatorService(repo)
pcbService.calculate(process.argv[2])