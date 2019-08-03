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

export interface SimpleMessage {
  type: string;
  text: string;
  language: string;
  condition: string;
}

export interface Tag {
  name: string;
  isAddition: boolean;
  condition: string;
}

export interface MessageContent {
  text: string;
  language: string;
}

export interface File {
  name: string;
  display: string;
  condition: string;
}

export interface PluginCleaningData {
  crc: number;
  util: string;
  itm: number;
  udr: number;
  nav: number;
  info: MessageContent[];
}

export interface ModLocation {
  name: string;
  link: string;
}

export interface PluginMetadata {
  name: string;
  enabled: boolean;
  after: File[];
  req: File[];
  inc: File[];
  msg: SimpleMessage[];
  tag: Tag[];
  dirty: PluginCleaningData[];
  clean: PluginCleaningData[];
  url: ModLocation[];

  group?: string;
}

export interface DerivedPluginMetadata {
  name: string;
  isActive: boolean;
  isDirty: boolean;
  isEmpty: boolean;
  isMaster: boolean;
  isLightMaster: boolean;
  loadsArchive: boolean;
  messages: SimpleMessage[];
  suggestedTags: Tag[];
  currentTags: Tag[];

  version?: string;
  crc?: number;
  group?: string;
  loadOrderIndex?: number;
  cleanedWith?: string;
  masterlist?: PluginMetadata;
  userlist?: PluginMetadata;
}
