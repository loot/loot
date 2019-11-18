import { mocked } from 'ts-jest/utils';
import {
  getVersion,
  getInitErrors,
  getConflictingPlugins
} from '../../../../gui/html/js/query';

describe('query()', () => {
  beforeAll(() => {
    window.cefQuery = jest
      .fn()
      .mockImplementation(({ request, onSuccess, onFailure }) => {
        if (request === '{"name":"getVersion"}') {
          onFailure(-1, 'error message');
        } else if (request === '{"name":"getInitErrors"}') {
          onSuccess('{"errors": []}');
        } else {
          onSuccess('{"generalMessages": [], "plugins": []}');
        }
        return 0;
      });
  });

  beforeEach(() => {
    mocked(window.cefQuery).mockClear();
  });

  test('should succeed if a request name is passed', () =>
    getInitErrors().then(errors => {
      expect(mocked(window.cefQuery).mock.calls.length).toBe(1);
      expect(errors.length).toBe(0);
    }));

  test('should succeed if a request name and arguments are passed', () =>
    getConflictingPlugins('plugin.esp').then(response => {
      expect(mocked(window.cefQuery).mock.calls.length).toBe(1);
      expect(response.generalMessages.length).toBe(0);
      expect(response.plugins.length).toBe(0);
    }));

  test('should fail with an Error object when an error occurs', () =>
    getVersion().catch(error => {
      expect(mocked(window.cefQuery).mock.calls.length).toBe(1);
      expect(error).toEqual(new Error('error message'));
    }));
});
