import { mocked } from 'ts-jest/utils';
import query from '../../../../gui/html/js/query';

describe('query()', () => {
  beforeAll(() => {
    window.cefQuery = jest
      .fn()
      .mockImplementation(({ request, onSuccess, onFailure }) => {
        if (request === '{"name":"CopyContent"}') {
          onFailure(-1, 'error message');
        } else {
          onSuccess();
        }
        return 0;
      });
  });

  beforeEach(() => {
    mocked(window.cefQuery).mockClear();
  });

  test('should return a promise', () =>
    query('test').then(() => {
      expect(mocked(window.cefQuery).mock.calls.length).toBe(1);
    }));

  test('should succeed if a request name is passed', () =>
    query('discardUnappliedChanges').then(() => {
      expect(mocked(window.cefQuery).mock.calls.length).toBe(1);
    }));

  test('should succeed if a request name and arguments are passed', () =>
    query('copyContent', { content: {} }).then(() => {
      expect(mocked(window.cefQuery).mock.calls.length).toBe(1);
    }));

  test('should fail with an Error object when an error occurs', () =>
    query('copyContent').catch(error => {
      expect(mocked(window.cefQuery).mock.calls.length).toBe(1);
      expect(error).toEqual(new Error('error message'));
    }));
});
