import * as marked from 'marked';
import { Game, Group, Language } from '../interfaces';

export function createGameItem(game: Game): HTMLElement {
  const menuItem = document.createElement('paper-item');
  menuItem.setAttribute('value', game.folder);
  menuItem.textContent = game.name;

  return menuItem;
}

export function createLanguageItem(language: Language): HTMLElement {
  const item = document.createElement('paper-item');
  item.setAttribute('value', language.locale);
  item.textContent = language.name;

  return item;
}

export function createThemeItem(theme: string): HTMLElement {
  const item = document.createElement('paper-item');
  item.setAttribute('value', theme);
  item.textContent = theme;

  return item;
}

export function createMessageItem(type: string, content: string): HTMLElement {
  const li = document.createElement('li');
  li.className = type;
  /* Use the Marked library for Markdown formatting support. */
  li.innerHTML = marked(content);

  return li;
}

export function createGameTypeItem(gameType: string): HTMLElement {
  const item = document.createElement('paper-item');
  item.setAttribute('value', gameType);
  item.textContent = gameType;

  return item;
}

export function createGroupItem(group: Group): HTMLElement {
  const item = document.createElement('paper-item');
  item.setAttribute('value', group.name);
  item.textContent = group.name;

  return item;
}
