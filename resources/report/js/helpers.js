function processCefError(err) {
    /* Error.stack seems to be Chromium-specific. It gives a lot more useful
       info than just the error message. Also, this can be used to catch any
       promise errors, not just CEF errors. */
    console.log(err.stack);
    closeProgressDialog();
    showMessageBox(l10n.jed.translate('Error').fetch(), err.message);
}

function showElement(element) {
    if (element != null) {
        element.classList.toggle('hidden', false);
    }
}
function hideElement(element) {
    if (element != null) {
        element.classList.toggle('hidden', true);
    }
}
function toast(text) {
    var toast = document.getElementById('toast');
    toast.text = text;
    toast.show();
}
function showMessageDialog(title, text, positiveText, closeCallback) {
    var dialog = document.createElement('loot-message-dialog');
    dialog.setButtonText(positiveText, l10n.jed.translate('Cancel').fetch());
    dialog.showModal(title, text, closeCallback);
    document.body.appendChild(dialog);
}
function showMessageBox(title, text) {
    var dialog = document.createElement('loot-message-dialog');
    dialog.setButtonText(l10n.jed.translate('OK').fetch());
    dialog.showModal(title, text);
    document.body.appendChild(dialog);
}

function showProgress(message) {
    var progressDialog = document.getElementById('progressDialog');
    if (message) {
        progressDialog.getElementsByTagName('p')[0].textContent = message;
    }
    if (!progressDialog.opened) {
        progressDialog.showModal();
    }
}
function closeProgressDialog() {
    var progressDialog = document.getElementById('progressDialog');
    if (progressDialog.opened) {
        progressDialog.close();
    }
}
function handleUnappliedChangesClose(change) {
    showMessageDialog('', l10n.jed.translate('You have not yet applied or cancelled your %s. Are you sure you want to quit?').fetch(change), l10n.jed.translate('Quit').fetch(), function(result){
        if (result) {
            /* Cancel any sorting and close any editors. Cheat by sending a
               cancelSort query for as many times as necessary. */
            var queries = [];
            var numQueries = 0;
            if (!document.getElementById('applySortButton').classList.contains('hidden')) {
                numQueries += 1;
            }
            numQueries += document.body.getAttribute('data-editors');
            for (var i = 0; i < numQueries; ++i) {
                queries.push(loot.query('cancelSort'));
            }
            Promise.all(queries).then(function(){
                window.close();
            }).catch(processCefError);
        }
    });
}

