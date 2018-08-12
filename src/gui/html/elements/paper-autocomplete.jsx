import React from 'react';
import ReactDOM from 'react-dom';
import PropTypes from 'prop-types';
import Autosuggest from 'react-autosuggest';

function getSuggestionValue(suggestion) {
  return suggestion;
}

function renderSuggestion(suggestion) {
  return <paper-item>{suggestion}</paper-item>;
}

class Autocomplete extends React.Component {
  constructor(props) {
    super();

    this.suggestionsSource = props.source;
    this.input = React.createRef();
    this.ironInput = React.createRef();
    this.state = {
      disabled: false,
      value: '',
      suggestions: []
    };
  }

  get value() {
    return this.state.value;
  }

  set value(value) {
    this.setState({ value });
  }

  set disabled(value) {
    this.setState({ disabled: value });
  }

  validate() {
    return this.ironInput.validate();
  }

  getSuggestions(value) {
    const inputValue = value.trim().toLowerCase();

    if (inputValue.length === 0) {
      return [];
    }

    return this.suggestionsSource.filter(
      suggestion =>
        suggestion.toLowerCase().slice(0, inputValue.length) === inputValue
    );
  }

  onChange(event, { newValue }) {
    this.setState({
      value: newValue
    });
  }

  onSuggestionsFetchRequested({ value }) {
    this.setState({
      suggestions: this.getSuggestions(value)
    });
  }

  onSuggestionsClearRequested() {
    this.setState({
      suggestions: []
    });
  }

  renderInputComponent(inputProps) {
    return (
      <paper-input-container
        disabled={inputProps.disabled ? true : null}
        auto-validate
        no-label-float
      >
        <iron-input
          slot="input"
          auto-validate
          bind-value={inputProps.value}
          ref={element => {
            this.ironInput = element;
          }}
        >
          <input required {...inputProps} />
        </iron-input>
        <paper-input-error slot="add-on">
          A value is required.
        </paper-input-error>
      </paper-input-container>
    );
  }

  render() {
    const { disabled, value, suggestions } = this.state;
    const inputProps = {
      disabled,
      value,
      onChange: (event, args) => this.onChange(event, args)
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
        // renderSuggestionsContainer={renderSuggestionsContainer}
        inputProps={inputProps}
      />
    );
  }
}

Autocomplete.propTypes = {
  source: PropTypes.arrayOf(PropTypes.string).isRequired
};

export default class PaperAutocomplete extends HTMLElement {
  constructor() {
    super();

    this.autocomplete = React.createRef();
  }

  connectedCallback() {
    const mountPoint = document.createElement('span');
    this.appendChild(mountPoint);

    const suggestionsSource = JSON.parse(this.getAttribute('source'));

    ReactDOM.render(
      <Autocomplete ref={this.autocomplete} source={suggestionsSource} />,
      mountPoint
    );
  }

  get value() {
    return this.autocomplete.current.value;
  }

  set value(value) {
    this.autocomplete.current.value = value;
  }

  set disabled(value) {
    this.autocomplete.current.disabled = value;
  }

  validate() {
    return this.autocomplete.current.validate();
  }
}

customElements.define('paper-autocomplete', PaperAutocomplete);
