export default function query(requestName, payload) {
  if (!requestName) {
    throw new Error('No request name passed');
  }

  return new Promise((resolve, reject) => {
    window.cefQuery({
      request: JSON.stringify(Object.assign({ name: requestName }, payload)),
      persistent: false,
      onSuccess: resolve,
      onFailure: (errorCode, errorMessage) => {
        reject(new Error(errorMessage));
      }
    });
  });
}
