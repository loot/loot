import { show, enable, setUIState } from './dom';

enum ApplicationState {
  Default,
  Sorting,
  Editing
}

export default class State {
  private currentState: ApplicationState;

  public constructor() {
    this.currentState = ApplicationState.Default;
  }

  public isInDefaultState(): boolean {
    return this.currentState === ApplicationState.Default;
  }

  public isInEditingState(): boolean {
    return this.currentState === ApplicationState.Editing;
  }

  public isInSortingState(): boolean {
    return this.currentState === ApplicationState.Sorting;
  }

  public enterSortingState(): void {
    if (this.isInSortingState()) {
      return;
    }
    if (this.isInEditingState()) {
      throw new Error('Cannot enter sorting state from editing state');
    }

    /* Hide the masterlist update buttons, and display the accept and
    cancel sort buttons. */
    show('updateMasterlistButton', false);
    show('sortButton', false);
    show('applySortButton');
    show('cancelSortButton');

    /* Disable changing game. */
    enable('gameMenu', false);
    enable('refreshContentButton', false);

    setUIState('sorting');

    this.currentState = ApplicationState.Sorting;
  }

  public exitSortingState(): void {
    if (this.isInDefaultState()) {
      return;
    }
    if (this.isInEditingState()) {
      throw new Error('Cannot exit sorting state from editing state');
    }

    /* Show the masterlist update buttons, and hide the accept and
    cancel sort buttons. */
    show('updateMasterlistButton');
    show('sortButton');
    show('applySortButton', false);
    show('cancelSortButton', false);

    /* Enable changing game. */
    enable('gameMenu');
    enable('refreshContentButton');

    setUIState('default');

    this.currentState = ApplicationState.Default;
  }

  public enterEditingState(): void {
    if (this.isInEditingState()) {
      return;
    }
    if (this.isInSortingState()) {
      throw new Error('Cannot enter editing state from sorting state');
    }

    /* Disable the toolbar elements. */
    enable('wipeUserlistButton', false);
    enable('copyContentButton', false);
    enable('refreshContentButton', false);
    enable('settingsButton', false);
    enable('gameMenu', false);
    enable('updateMasterlistButton', false);
    enable('sortButton', false);

    setUIState('editing');

    this.currentState = ApplicationState.Editing;
  }

  public exitEditingState(): void {
    if (this.isInDefaultState()) {
      return;
    }
    if (this.isInSortingState()) {
      throw new Error('Cannot exit editing state from sorting state');
    }

    /* Re-enable toolbar elements. */
    enable('wipeUserlistButton');
    enable('copyContentButton');
    enable('refreshContentButton');
    enable('settingsButton');
    enable('gameMenu');
    enable('updateMasterlistButton');
    enable('sortButton');

    setUIState('default');

    this.currentState = ApplicationState.Default;
  }
}
