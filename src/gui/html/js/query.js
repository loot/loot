'use strict';

(function exportModule(root, factory) {
  if (typeof define === 'function' && define.amd) {
    // AMD. Register as an anonymous module.
    define(['bower_components/lodash/dist/lodash.core.min'], factory);
  } else {
    // Browser globals
    root.loot = root.loot || {};
    root.loot.query = factory(root._);
  }
})(this, _ => (requestName, payload) => {
  if (!requestName) {
    throw new Error('No request name passed');
  }
  const request = {
    name: requestName
  };
  if (payload) {
    if (Array.isArray(payload)) {
      request.pluginNames = payload;
    } else if (_.isString(payload)) {
      request.targetName = payload;
    } else if (payload.name) {
      request.filter = payload;
    } else if (payload.messages) {
      request.content = payload;
    } else if (
      Object.prototype.hasOwnProperty.call(payload, 'enableDebugLogging')
    ) {
      request.settings = payload;
    } else if (payload.metadata) {
      request.editorState = payload;
    }
  }

  return new Promise((resolve, reject) => {
    window.cefQuery({
      request: JSON.stringify(request),
      persistent: false,
      onSuccess: resolve,
      onFailure: (errorCode, errorMessage) => {
        reject(new Error(errorMessage));
      }
    });
  });
});
