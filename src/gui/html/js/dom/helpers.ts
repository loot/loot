import { PaperInputElement } from '@polymer/paper-input/paper-input';
import { PaperIconButtonElement } from '@polymer/paper-icon-button';

export function incrementCounterText(
  elementId: string,
  increment: number
): void {
  const element = document.getElementById(elementId);
  if (element === null) {
    return;
  }

  element.textContent = (
    parseInt(element.textContent || '0', 10) + increment
  ).toString();
}

export function getTextAsInt(elementId: string): number {
  const element = document.getElementById(elementId);
  if (element === null || element.textContent === null) {
    return 0;
  }

  return parseInt(element.textContent, 10);
}

export function getElementById(elementId: string): HTMLElement {
  const element = document.getElementById(elementId);
  if (element === null) {
    throw new Error(`Expected to get an element with ID ${elementId}`);
  }

  return element;
}

export function getShadowElementById(
  shadowRoot: ShadowRoot,
  id: string
): HTMLElement {
  const element = shadowRoot.getElementById(id);
  if (element === null) {
    throw new Error(`Expected to get an element with ID ${id}`);
  }

  return element;
}

export function setTextContent(elementId: string, value: number): void {
  getElementById(elementId).textContent = value.toString();
}

export function getAttribute(element: Element, attribute: string): string {
  const value = element.getAttribute(attribute);
  if (value === null) {
    throw new Error(`Expected element to have an attribute named ${attribute}`);
  }

  return value;
}

export function querySelector(
  element: Element | Document | DocumentFragment,
  selector: string
): Element {
  const selected = element.querySelector(selector);

  if (selected === null) {
    throw new Error(
      `Expected element to have a child matching the selector ${selector}`
    );
  }

  return selected;
}

export function isPaperInput(
  element: Element
): element is Element & PaperInputElement {
  return element.tagName === 'PAPER-INPUT';
}

export function isPaperIconButton(
  element: Element
): element is PaperIconButtonElement {
  return element.tagName === 'PAPER-ICON-BUTTON';
}
