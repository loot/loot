/* Mock the DOM interactions */
window.loot = window.loot || {};

loot.DOM = {
  elementShownStates: new Map(),
  elementEnabledStates: new Map(),

  show(elementId, showElement = true) {
    this.elementShownStates.set(elementId, showElement);
  },

  enable(elementId, enableElement = true) {
    this.elementEnabledStates.set(elementId, enableElement);
  },
};
