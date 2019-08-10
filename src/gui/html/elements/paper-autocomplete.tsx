import * as React from 'react';
import * as ReactDOM from 'react-dom';
import * as Autosuggest from 'react-autosuggest';
import { IronInputElement } from '@polymer/iron-input';

// For some reason using the actual types of the Polymer elements results in
// type errors in paper-autocomplete.tsx, so use any instead.
/* eslint-disable @typescript-eslint/no-explicit-any */
declare global {
  // eslint-disable-next-line @typescript-eslint/no-namespace
  namespace JSX {
    interface IntrinsicElements {
      'paper-input-container': any; // PaperInputContainerElement;
      'paper-item': any; // PaperItemElement;
      'paper-input-error': any; // PaperInputErrorElement;
      'iron-input': any; // IronInputElement;
    }
  }
}
/* eslint-enable @typescript-eslint/no-explicit-any */

function getSuggestionValue(suggestion: string): string {
  return suggestion;
}

function renderSuggestion(suggestion: string): JSX.Element {
  return <paper-item>{suggestion}</paper-item>;
}

interface AutocompleteProps {
  source: string[];
}

interface AutocompleteState {
  disabled: boolean;
  suggestions: string[];
  value: string;
}

class Autocomplete extends React.Component<
  AutocompleteProps,
  AutocompleteState
> {
  private ironInput: React.RefObject<IronInputElement>;

  public constructor(props: AutocompleteProps) {
    super(props);

    this.ironInput = React.createRef();
    this.state = {
      disabled: false,
      value: '',
      suggestions: []
    };
  }

  public get value(): string {
    return this.state.value;
  }

  public set value(value) {
    this.setState({ value });
  }

  public set disabled(value: boolean) {
    this.setState({ disabled: value });
  }

  public validate(): boolean {
    if (this.ironInput.current === null) {
      throw new Error('Expected current iron-input ref to be non-null');
    }

    return this.ironInput.current.validate();
  }

  public getSuggestions(value: string): string[] {
    const inputValue = value.trim().toLowerCase();

    if (inputValue.length === 0) {
      return [];
    }

    return this.props.source.filter(
      suggestion =>
        suggestion.toLowerCase().slice(0, inputValue.length) === inputValue
    );
  }

  public onChange(
    // eslint-disable-next-line @typescript-eslint/no-explicit-any
    _event: React.FormEvent<any>,
    params: Autosuggest.ChangeEvent
  ): void {
    this.setState({
      value: params.newValue
    });
  }

  public onSuggestionsFetchRequested(
    params: Autosuggest.SuggestionsFetchRequestedParams
  ): void {
    this.setState({
      suggestions: this.getSuggestions(params.value)
    });
  }

  public onSuggestionsClearRequested(): void {
    this.setState({
      suggestions: []
    });
  }

  public renderInputComponent(
    inputProps: Autosuggest.InputProps<string>
  ): JSX.Element {
    const { onChange, ...otherProps } = inputProps;
    return (
      <paper-input-container
        disabled={otherProps.disabled ? true : null}
        auto-validate
        no-label-float
      >
        <iron-input
          slot="input"
          auto-validate
          bind-value={otherProps.value}
          ref={this.ironInput}
        >
          <input
            required
            onChange={evt => {
              onChange(evt, {
                newValue: evt.currentTarget.value,
                method: 'type'
              });
            }}
            {...otherProps}
          />
        </iron-input>
        <paper-input-error slot="add-on">
          A value is required.
        </paper-input-error>
      </paper-input-container>
    );
  }

  public render(): JSX.Element {
    const { disabled, value, suggestions } = this.state;
    const inputProps = {
      disabled,
      value,
      onChange: (
        // eslint-disable-next-line @typescript-eslint/no-explicit-any
        event: React.FormEvent<any>,
        params: Autosuggest.ChangeEvent
      ) => this.onChange(event, params)
    };

    return (
      <Autosuggest
        suggestions={suggestions}
        onSuggestionsFetchRequested={args =>
          this.onSuggestionsFetchRequested(args)
        }
        onSuggestionsClearRequested={() => this.onSuggestionsClearRequested()}
        getSuggestionValue={getSuggestionValue}
        renderSuggestion={renderSuggestion}
        renderInputComponent={props => this.renderInputComponent(props)}
        inputProps={inputProps}
      />
    );
  }
}

export default class PaperAutocomplete extends HTMLElement {
  private autocomplete: React.RefObject<Autocomplete>;

  public constructor() {
    super();

    this.autocomplete = React.createRef();
  }

  public connectedCallback(): void {
    const mountPoint = document.createElement('span');
    this.appendChild(mountPoint);

    const sourceAttribute = this.getAttribute('source') || '[]';
    const suggestionsSource = JSON.parse(sourceAttribute);

    ReactDOM.render(
      <Autocomplete ref={this.autocomplete} source={suggestionsSource} />,
      mountPoint
    );
  }

  public get value(): string {
    if (this.autocomplete.current === null) {
      throw new Error('Expected current autocomplete ref to be non-null');
    }

    return this.autocomplete.current.value;
  }

  public set value(value) {
    if (this.autocomplete.current === null) {
      throw new Error('Expected current autocomplete ref to be non-null');
    }

    this.autocomplete.current.value = value;
  }

  public set disabled(value: boolean) {
    if (this.autocomplete.current === null) {
      throw new Error('Expected current autocomplete ref to be non-null');
    }

    this.autocomplete.current.disabled = value;
  }

  public validate(): boolean {
    if (this.autocomplete.current === null) {
      throw new Error('Expected current autocomplete ref to be non-null');
    }

    return this.autocomplete.current.validate();
  }
}

customElements.define('paper-autocomplete', PaperAutocomplete);
