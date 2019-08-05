import Translator from '../js/translator';
import Filters from '../js/filters';
import Game from '../js/game';
import State from '../js/state';
import { LootSettings, LootVersion } from '../js/interfaces';

export default interface Loot {
  filters?: Filters;
  game?: Game;
  l10n?: Translator;
  settings?: LootSettings;
  state?: State;
  installedGames?: string[];
  version?: LootVersion;
  showProgress?: (text: string) => void;
  onQuit?: () => void;
}

declare global {
  interface Window {
    loot: Loot;
  }
}
