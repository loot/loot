import Translator from '../js/translator';
import Filters from '../js/filters';
import Game from '../js/game';
import State from '../js/state';
import { LootSettings } from '../js/interfaces';

declare global {
  interface Window {
    loot: {
      filters?: Filters;
      game?: Game;
      l10n?: Translator;
      settings?: LootSettings;
      state?: State;
      installedGames?: string[];
      showProgress?: (text: string) => void;
      onQuit?: () => void;
    };
  }
}
