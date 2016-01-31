'use strict';
(function exportModule(root, factory) {
  if (typeof define === 'function' && define.amd) {
    // AMD. Register as an anonymous module.
    define([], factory);
  } else {
    // Browser globals
    root.loot = root.loot || {};
    root.loot.query = factory();
  }
}(this, () => {
  return (requestName, ...args) => {
    if (!requestName) {
      throw new Error('No request name passed');
    }
    let request;
    if (args.length === 0) {
      request = requestName;
    } else {
      request = JSON.stringify({
        name: requestName,
        args,
      });
    }
    return new Promise((resolve, reject) => {
      window.cefQuery({
        request,
        persistent: false,
        onSuccess: resolve,
        onFailure: (errorCode, errorMessage) => {
          reject(new Error(`Error code: ${errorCode}; ${errorMessage}`));
        },
      });
    });
  };
}));
