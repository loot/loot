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
}(this, () => {
  return class Dialog {
    static showProgress(text) {
      const progressDialog = document.getElementById('progressDialog');
      progressDialog.getElementsByTagName('p')[0].textContent = text;
      if (!progressDialog.opened) {
        progressDialog.showModal();
      }
    }

    static closeProgress() {
      const progressDialog = document.getElementById('progressDialog');
      if (progressDialog.opened) {
        progressDialog.close();
      }
    }

    static showMessage(title, text) {
      const dialog = document.createElement('loot-message-dialog');
      dialog.setDismissable(false);
      dialog.showModal(title, text);
      document.body.appendChild(dialog);
    }

    static askQuestion(title, text, confirmText, closeCallback) {
      const dialog = document.createElement('loot-message-dialog');
      dialog.setConfirmText(confirmText);
      dialog.showModal(title, text, closeCallback);
      document.body.appendChild(dialog);
    }

    static showNotification(text) {
      const toast = document.getElementById('toast');
      toast.text = text;
      toast.show();
    }
  };
}));
