import Translator from '../js/translator';

declare global {
  interface Window {
    loot: {
      l10n?: Translator;
      showProgress?: (text: string) => void;
      onQuit?: () => void;
    };
  }
}
