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
  fontFamily?: string;
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

export interface GameSettings {
  type: string;
  name: string;
  master: string;
  registry: string;
  folder: string;
  repo: string;
  branch: string;
  path: string;
  localPath: string;
}

export interface FilterStates {
  hideMessagelessPlugins: boolean;
  hideInactivePlugins: boolean;
  hideVersionNumbers: boolean;
  hideCRCs: boolean;
  hideBashTags: boolean;
  hideAllPluginMessages: boolean;
  hideNotes: boolean;
  hideDoNotCleanMessages: boolean;
}

export interface LootSettings {
  game: string;
  games: GameSettings[];
  lastVersion: string;
  language: string;
  languages: Language[];
  theme: string;
  enableDebugLogging: boolean;
  updateMasterlist: boolean;
  enableLootUpdateCheck: boolean;
  filters: FilterStates;
}

export interface PluginContent {
  name: string;
  crc: number;
  version: string;
  isActive: boolean;
  isEmpty: boolean;
  loadsArchive: boolean;
  isDirty: boolean;

  group: string;
  messages: SimpleMessage[];
  currentTags: Tag[];
  suggestedTags: Tag[];
}

export interface GameContent {
  messages: SimpleMessage[];
  plugins: PluginContent[];
}

export interface GameData {
  folder: string;
  generalMessages: SimpleMessage[];
  masterlist: Masterlist;
  groups: GameGroups;
  plugins: DerivedPluginMetadata[];
  bashTags: string[];
}

export interface Masterlist {
  revision: string;
  date: string;
}

export interface GameGroups {
  masterlist: RawGroup[];
  userlist: RawGroup[];
}

export interface MainContent {
  generalMessages: SimpleMessage[];
  plugins: DerivedPluginMetadata[];
}

export interface LootVersion {
  release: string;
  build: string;
}

export interface PluginLoadOrderIndex {
  name: string;
  loadOrderIndex?: number;
}

export interface PluginItemContentChangePayload {
  pluginId: string;
  group: string;
  isEditorOpen: boolean;
  hasUserEdits: boolean;
  loadOrderIndex?: number;
  isLightMaster: boolean;
}

export interface PluginTags {
  current: string;
  add: string;
  remove: string;
}

export interface TagRowData {
  name: string;
  type: string;
  condition: string;
}

export interface CleaningRowData {
  crc: string;
  itm: string;
  udr: string;
  nav: string;
  utility: string;
}
