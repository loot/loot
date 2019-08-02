declare global {
  interface Window {
    cefQuery: (query: object) => number;
  }
}

export default function query(
  requestName: string,
  payload: object
): Promise<string> {
  if (!requestName) {
    throw new Error('No request name passed');
  }

  return new Promise((resolve, reject): void => {
    window.cefQuery({
      request: JSON.stringify(Object.assign({ name: requestName }, payload)),
      persistent: false,
      onSuccess: resolve,
      onFailure: (_errorCode: number, errorMessage: string): void => {
        reject(new Error(errorMessage));
      }
    });
  });
}
