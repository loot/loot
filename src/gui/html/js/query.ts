interface CefQueryParameters {
  request: string;
  persistent: boolean;
  onSuccess: (response: string) => void;
  onFailure: (errorCode: number, errorMessage: string) => void;
}

declare global {
  interface Window {
    cefQuery: (query: CefQueryParameters) => number;
  }
}

export default function query(
  requestName: string,
  payload?: object
): Promise<string> {
  if (!requestName) {
    throw new Error('No request name passed');
  }

  return new Promise((resolve, reject): void => {
    window.cefQuery({
      request: JSON.stringify(Object.assign({ name: requestName }, payload)),
      persistent: false,
      onSuccess: resolve,
      onFailure: (_errorCode, errorMessage) => {
        reject(new Error(errorMessage));
      }
    });
  });
}
