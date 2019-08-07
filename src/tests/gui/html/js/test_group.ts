import mergeGroups from '../../../../gui/html/js/group';
import { RawGroup } from '../../../../gui/html/js/interfaces';

describe('mergeGroups', () => {
  let masterlist: RawGroup[];
  let userlist: RawGroup[];

  beforeAll(() => {
    masterlist = [
      {
        name: 'A',
        after: []
      },
      {
        name: 'B',
        after: ['A']
      },
      {
        name: 'C',
        after: ['B']
      }
    ];

    userlist = [
      {
        name: 'C',
        after: ['A', 'B']
      },
      {
        name: 'D',
        after: ['C']
      },
      {
        name: 'E',
        after: []
      }
    ];
  });

  test('should not modify input masterlist object', () => {
    mergeGroups(masterlist, []);

    expect(masterlist).toEqual([
      {
        name: 'A',
        after: []
      },
      {
        name: 'B',
        after: ['A']
      },
      {
        name: 'C',
        after: ['B']
      }
    ]);
  });

  test('should turn masterlist group after entry strings into objects', () => {
    const merged = mergeGroups(masterlist, []);

    expect(merged[1].after[0]).toEqual({
      name: 'A',
      isUserAdded: false,
      after: []
    });
    expect(merged[2].after[0]).toEqual({
      name: 'B',
      isUserAdded: false,
      after: []
    });
  });

  test('should add isUserAdded false to masterlist groups', () => {
    const merged = mergeGroups(masterlist, []);

    expect(merged[0].isUserAdded).toBe(false);
    expect(merged[1].isUserAdded).toBe(false);
    expect(merged[2].isUserAdded).toBe(false);
  });

  test('should merge masterlist and userlist groups', () => {
    const merged = mergeGroups(masterlist, userlist);

    expect(merged.length).toBe(5);
    expect(merged[0].name).toBe('A');
    expect(merged[1].name).toBe('B');
    expect(merged[2].name).toBe('C');
    expect(merged[3].name).toBe('D');
    expect(merged[4].name).toBe('E');
  });

  test('should turn userlist group after entry strings into objects', () => {
    const merged = mergeGroups(masterlist, userlist);

    expect(merged[3].after[0]).toEqual({
      name: 'C',
      isUserAdded: true,
      after: []
    });
  });

  test('should merge masterlist and userlist after groups', () => {
    const merged = mergeGroups(masterlist, userlist);

    expect(merged[2].after.length).toBe(2);
    expect(merged[2].after[0]).toEqual({
      name: 'A',
      isUserAdded: true,
      after: []
    });
    expect(merged[2].after[1]).toEqual({
      name: 'B',
      isUserAdded: false,
      after: []
    });
  });

  test('should not add isUserAdded true to merged userlist groups', () => {
    const merged = mergeGroups(masterlist, userlist);

    expect(merged[2].isUserAdded).toBe(false);
  });

  test('should add isUserAdded true to new userlist groups', () => {
    const merged = mergeGroups(masterlist, userlist);

    expect(merged[4].isUserAdded).toBe(true);
  });
});
