import { PaperDialogElement } from '@polymer/paper-dialog';
import { PaperToastElement } from '@polymer/paper-toast';
import LootMessageDialog from '../elements/loot-message-dialog';
import { getElementById } from './dom/helpers';

export function showProgress(text: string): void {
  const progressDialog = getElementById('progressDialog') as PaperDialogElement;
  progressDialog.getElementsByTagName('p')[0].textContent = text;
  if (!progressDialog.opened) {
    progressDialog.open();
  } else {
    progressDialog.refit();
  }
}

export function closeProgress(): void {
  const progressDialog = getElementById('progressDialog') as PaperDialogElement;
  if (progressDialog.opened) {
    progressDialog.refit();
    progressDialog.close();
  }
}

export function showMessage(title: string, text: string): void {
  const dialog = document.createElement(
    'loot-message-dialog'
  ) as LootMessageDialog;
  document.body.appendChild(dialog);
  dialog.setDismissable(false);
  dialog.showModal(title, text);
}

export function askQuestion(
  title: string,
  text: string,
  confirmText: string,
  closeCallback: (confirmed: boolean) => void
): void {
  const dialog = document.createElement(
    'loot-message-dialog'
  ) as LootMessageDialog;
  document.body.appendChild(dialog);
  dialog.setConfirmText(confirmText);
  dialog.showModal(title, text, closeCallback);
}

export function showNotification(text: string): void {
  const toast = getElementById('toast') as PaperToastElement;
  if (toast.opened) {
    toast.hide();
  }
  toast.show(text);
}
