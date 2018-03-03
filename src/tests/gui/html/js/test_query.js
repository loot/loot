import * as _ from 'lodash';
import query from '../../../../gui/html/js/query.js';

describe('query()', () => {
  beforeAll(() => {
    window._ = _;

    window.cefQuery = jest
      .fn()
      .mockImplementation(({ request, onSuccess, onFailure }) => {
        if (request === '{"name":"CopyContent"}') {
          onFailure(-1, 'error message');
        } else {
          onSuccess();
        }
      });
  });

  beforeEach(() => {
    window.cefQuery.mockClear();
  });

  test('should throw if no arguments are passed', () => {
    expect(() => {
      query();
    }).toThrow('No request name passed');
  });

  test('should return a promise', () =>
    query('test').then(() => {
      expect(window.cefQuery.mock.calls.length).toBe(1);
    }));

  test('should succeed if a request name is passed', () =>
    query('discardUnappliedChanges').then(() => {
      expect(window.cefQuery.mock.calls.length).toBe(1);
    }));

  test('should succeed if a request name and arguments are passed', () =>
    query('copyContent', { messages: {} }).then(() => {
      expect(window.cefQuery.mock.calls.length).toBe(1);
    }));

  test('should fail with an Error object when an error occurs', () =>
    query('copyContent').catch(error => {
      expect(window.cefQuery.mock.calls.length).toBe(1);
      expect(error).toEqual(new Error('error message'));
    }));
});
