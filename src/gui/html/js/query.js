// Depends on the lodash library, which isn't available as an ES2015 module.
import * as _ from 'lodash/core.min';

export default function query(requestName, payload) {
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
    } else if (payload.userGroups) {
      request.userGroups = payload.userGroups;
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
}
