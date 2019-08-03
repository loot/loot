export interface Game {
  name: string;
  folder: string;
}

export interface Group {
  name: string;
}

export interface RawGroup extends Group {
  name: string;
  after: string[];
}

export interface SourcedGroup extends Group {
  name: string;
  after: SourcedGroup[];
  isUserAdded: boolean;
}

export interface Language {
  name: string;
  locale: string;
}
