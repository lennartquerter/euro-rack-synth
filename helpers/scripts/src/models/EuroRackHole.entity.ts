export interface EuroRackHoleEntity {
    label: string;
    distanceFromTopInMM: number;
    distanceFromRightInMM: number;
    type: 'jack' | 'led' | 'pot' | 'sw';
}