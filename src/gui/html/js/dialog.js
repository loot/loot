'use strict';

(function exportModule(root, factory) {
  if (typeof define === 'function' && define.amd) {
    // AMD. Register as an anonymous module.
    define([], factory);
  } else {
    // Browser globals
    root.loot = root.loot || {};
    root.loot.Dialog = factory();
  }
})(
  this,
  () =>
    class {
      static showProgress(text) {
        const progressDialog = document.getElementById('progressDialog');
        progressDialog.getElementsByTagName('p')[0].textContent = text;
        if (!progressDialog.opened) {
          progressDialog.open();
        } else {
          progressDialog.refit();
        }
      }

      static closeProgress() {
        const progressDialog = document.getElementById('progressDialog');
        if (progressDialog.opened) {
          progressDialog.refit();
          progressDialog.close();
        }
      }

      static showMessage(title, text) {
        const dialog = document.createElement('loot-message-dialog');
        document.body.appendChild(dialog);
        dialog.setDismissable(false);
        dialog.showModal(title, text);
      }

      static askQuestion(title, text, confirmText, closeCallback) {
        const dialog = document.createElement('loot-message-dialog');
        document.body.appendChild(dialog);
        dialog.setConfirmText(confirmText);
        dialog.showModal(title, text, closeCallback);
      }

      static showNotification(text) {
        const toast = document.getElementById('toast');
        if (toast.opened) {
          toast.hide();
        }
        toast.show(text);
      }
    }
);
