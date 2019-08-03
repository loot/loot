// Depends on the loot.l10n global.
import { closeProgress, showMessage } from './dialog';

export default function handlePromiseError(error: Error): void {
  /* Error.stack seems to be Chromium-specific. */
  console.error(error.stack); // eslint-disable-line no-console
  closeProgress();
  showMessage(window.loot.l10n.translate('Error'), error.message);
}
