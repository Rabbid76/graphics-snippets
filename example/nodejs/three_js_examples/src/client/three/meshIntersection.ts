// @ts-ignore
import * as MeshUtility from '../../../buildWASM/MeshUtility.js';

let meshUtility: any | null;
export const intiMeshUtility = async () => {
    if (meshUtility !== undefined) {
        return;
    }
    meshUtility = await MeshUtility();
    console.log('MeshGenerator version: ' + meshUtility.getVersion());
    meshUtility.setIOContext({
        log: (message: string) => console.log(message)
    });
};

export const calculateMeshIntersection = (mesh0: any, mesh1: any): any => {
    if (meshUtility === undefined) {
        return null;
    }
    try {
        return meshUtility.calculateMeshIntersection(mesh0, mesh1);
    } catch (error) {
        console.error(error);
    }
    return null;
}

export const calculateMeshMinusAB = (mesh0: any, mesh1: any): any => {
    if (meshUtility === undefined) {
        return null;
    }
    try {
        return meshUtility.meshOperatorMinusAB(mesh0, mesh1);
    } catch (error) {
        console.error(error);
    }
    return null;
}

export const calculateMeshOrAB = (mesh0: any, mesh1: any): any => {
    if (meshUtility === undefined) {
        return null;
    }
    try {
        return meshUtility.meshOperatorOrAB(mesh0, mesh1);
    } catch (error) {
        console.error(error);
    }
    return null;
}

export const calculateMeshAndAB = (mesh0: any, mesh1: any): any => {
    if (meshUtility === undefined) {
        return null;
    }
    try {
        return meshUtility.meshOperatorAndAB(mesh0, mesh1);
    } catch (error) {
        console.error(error);
    }
    return null;
}