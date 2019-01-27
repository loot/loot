function transformGroupAfter(afterGroups, isUserAdded) {
  if (afterGroups === undefined) {
    return [];
  }

  return afterGroups
    .map(groupName => ({
      name: groupName,
      isUserAdded
    }))
    .sort((a, b) => a.name.localeCompare(b.name));
}

function transformGroup(group, isUserAdded) {
  const after = transformGroupAfter(group.after, isUserAdded);

  return Object.assign({}, group, { isUserAdded, after });
}

function mergeGroupAfters(groupAfter1, groupAfter2) {
  return groupAfter2
    .reduce((groups, group) => {
      if (groups.find(after => after.name === group.name) === undefined) {
        groups.push(group);
      }
      return groups;
    }, groupAfter1.slice())
    .sort((a, b) => a.name.localeCompare(b.name));
}

export default function mergeGroups(masterlistGroups, userGroups) {
  const groups = masterlistGroups.map(group => transformGroup(group, false));

  userGroups.forEach(userGroup => {
    const existingGroup = groups.find(group => group.name === userGroup.name);
    if (existingGroup === undefined) {
      groups.push(transformGroup(userGroup, true));
    } else {
      const userAfter = transformGroupAfter(userGroup.after, true);
      existingGroup.after = mergeGroupAfters(existingGroup.after, userAfter);
    }
  });

  return groups.sort((a, b) => a.name.localeCompare(b.name));
}
