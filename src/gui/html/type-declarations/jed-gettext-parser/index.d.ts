declare module 'jed-gettext-parser' {
  import { LocaleData } from 'jed';

  interface MoParser {
    parse: (buffer: ArrayBuffer) => Promise<LocaleData>;
  }

  export default class JedGettextParser {
    public static mo: MoParser;
  }
}
