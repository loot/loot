declare module 'jed' {
  export interface DomainConfig {
    domain: string;
    lang: string;
    // eslint-disable-next-line camelcase
    plural_forms: string;
  }

  export interface DomainData {
    [key: string]: string[] | DomainConfig;
  }

  export interface LocaleData {
    [domain: string]: DomainData;
  }

  interface Options {
    // eslint-disable-next-line camelcase
    locale_data: LocaleData;
    domain: string;
  }

  interface Chain {
    fetch: (...substitutions: (string | object)[]) => string;
  }

  export class Jed {
    public constructor(options: Options);

    public translate(text: string): Chain;
  }
}
