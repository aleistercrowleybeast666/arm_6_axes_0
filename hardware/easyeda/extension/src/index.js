import design from '../../../design/controller_revA.json';
import {Design_Generate} from './generator.js';

export async function generateRevA() {
    try { await Design_Generate(eda,design); }
    catch (error) {
        await eda.sys_Dialog.showInformationMessage(`Rev.A generation stopped: ${String(error)}\nNo ERC PASS is claimed. Any new partial project is retained for diagnosis.`);
        throw error;
    }
}
