export function showProgress(text) {
  const progressDialog = document.getElementById('progressDialog');
  progressDialog.getElementsByTagName('p')[0].textContent = text;
  if (!progressDialog.opened) {
    progressDialog.open();
  } else {
    progressDialog.refit();
  }
}

export function closeProgress() {
  const progressDialog = document.getElementById('progressDialog');
  if (progressDialog.opened) {
    progressDialog.refit();
    progressDialog.close();
  }
}

export function showMessage(title, text) {
  const dialog = document.createElement('loot-message-dialog');
  document.body.appendChild(dialog);
  dialog.setDismissable(false);
  dialog.showModal(title, text);
}

export function askQuestion(title, text, confirmText, closeCallback) {
  const dialog = document.createElement('loot-message-dialog');
  document.body.appendChild(dialog);
  dialog.setConfirmText(confirmText);
  dialog.showModal(title, text, closeCallback);
}

export function showNotification(text) {
  const toast = document.getElementById('toast');
  if (toast.opened) {
    toast.hide();
  }
  toast.show(text);
}
