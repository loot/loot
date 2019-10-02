import '@polymer/iron-icons/iron-icons';
import '@polymer/paper-icon-button';
import '@polymer/paper-input/paper-input';
import '@polymer/paper-input/paper-textarea';
import '@polymer/paper-item/paper-item';
import '@polymer/paper-tooltip';
import './paper-autocomplete';

const $documentContainer = document.createElement('div');
$documentContainer.setAttribute('style', 'display: none;');

$documentContainer.innerHTML = `<template id="fileRow">
  <tr>
    <td><paper-autocomplete error-message="A filename is required." class="name" required="" auto-validate="" no-label-float=""></paper-autocomplete></td>
    <td><paper-input class="display" no-label-float=""></paper-input></td>
    <td><paper-input class="condition" no-label-float=""></paper-input></td>
    <td>
      <span>
        <paper-icon-button class="delete" icon="delete"></paper-icon-button>
        <paper-tooltip position="left">Delete Row</paper-tooltip>
      </span>
    </td>
  </tr>
</template><template id="messageRow">
  <tr>
    <td style="width: 100px;">
      <loot-dropdown-menu class="type" value="say" no-label-float="">
        <paper-item value="say">Note</paper-item>
        <paper-item value="warn">Warning</paper-item>
        <paper-item value="error">Error</paper-item>
      </loot-dropdown-menu>
    </td>
    <td><paper-input error-message="A content string is required." class="text" required="" auto-validate="" no-label-float=""></paper-input></td>
    <td><paper-input class="condition" no-label-float=""></paper-input></td>
    <td>
      <loot-dropdown-menu class="language" no-label-float="">
        <!-- Language <option> elements go here. -->
      </loot-dropdown-menu>
    </td>
    <td>
      <span>
        <paper-icon-button class="delete" icon="delete"></paper-icon-button>
        <paper-tooltip position="left">Delete Row</paper-tooltip>
      </span>
    </td>
  </tr>
</template><template id="tagRow">
  <tr>
    <td style="width: 100px;">
      <loot-dropdown-menu class="type" value="add" no-label-float="">
        <paper-item value="add">Add</paper-item>
        <paper-item value="remove">Remove</paper-item>
      </loot-dropdown-menu>
    </td>
    <td><paper-autocomplete error-message="A name is required." class="name" required="" auto-validate="" no-label-float=""></paper-autocomplete></td>
    <td><paper-input class="condition" no-label-float=""></paper-input></td>
    <td>
      <span>
        <paper-icon-button class="delete" icon="delete"></paper-icon-button>
        <paper-tooltip position="left">Delete Row</paper-tooltip>
      </span>
    </td>
  </tr>
</template><template id="dirtyInfoRow">
  <tr>
    <td style="width: 104px;"><paper-input error-message="A CRC is required." class="crc" maxlength="8" minlength="1" pattern="[0-9A-Fa-f]{1,8}" required="" auto-validate="" no-label-float=""></paper-input></td>
    <td style="width: 48px;"><paper-input error-message="Values must be integers." class="itm" type="number" min="0" step="1" value="0" auto-validate="" no-label-float=""></paper-input></td>
    <td style="width: 48px;"><paper-input error-message="Values must be integers." class="udr" type="number" min="0" step="1" value="0" auto-validate="" no-label-float=""></paper-input></td>
    <td style="width: 48px;"><paper-input error-message="Values must be integers." class="nav" type="number" min="0" step="1" value="0" auto-validate="" no-label-float=""></paper-input></td>
    <td><paper-input error-message="A utility name is required." class="utility" required="" auto-validate="" no-label-float=""></paper-input></td>
    <td>
      <span>
        <paper-icon-button class="delete" icon="delete"></paper-icon-button>
        <paper-tooltip position="left">Delete Row</paper-tooltip>
      </span>
    </td>
  </tr>
</template><template id="cleanInfoRow">
  <tr>
    <td style="width: 92px;"><paper-input error-message="A CRC is required." class="crc" maxlength="8" minlength="1" pattern="[0-9A-Fa-f]{1,8}" required="" auto-validate="" no-label-float=""></paper-input></td>
    <td><paper-input error-message="A utility name is required." class="utility" required="" auto-validate="" no-label-float=""></paper-input></td>
    <td>
      <span>
        <paper-icon-button class="delete" icon="delete"></paper-icon-button>
        <paper-tooltip position="left">Delete Row</paper-tooltip>
      </span>
    </td>
  </tr>
</template><template id="locationRow">
  <tr>
    <td><paper-input error-message="A link is required." class="link" required="" auto-validate="" no-label-float=""></paper-input></td>
    <td><paper-input class="name" no-label-float=""></paper-input></td>
    <td>
      <span>
        <paper-icon-button class="delete" icon="delete"></paper-icon-button>
        <paper-tooltip position="left">Delete Row</paper-tooltip>
      </span>
    </td>
  </tr>
</template><template id="gameRow">
  <tr>
    <td><paper-input error-message="A name is required." class="name" required="" auto-validate="" no-label-float=""></paper-input></td>
    <td>
      <loot-dropdown-menu class="type" no-label-float="">
        <!-- Game <option> elements go here. -->
      </loot-dropdown-menu>
    </td>
    <td><paper-input error-message="A folder is required." class="folder" required="" auto-validate="" no-label-float=""></paper-input></td>
    <td><paper-input class="master" no-label-float=""></paper-input></td>
    <td><paper-input class="repo" no-label-float=""></paper-input></td>
    <td><paper-input class="branch" no-label-float=""></paper-input></td>
    <td><paper-input class="path" no-label-float=""></paper-input></td>
    <td hidden=""><paper-input class="localPath" no-label-float=""></paper-input></td>
    <td><paper-input class="registry" no-label-float=""></paper-input></td>
    <td>
      <span>
        <paper-icon-button class="delete" icon="delete"></paper-icon-button>
        <paper-tooltip position="left">Delete Row</paper-tooltip>
      </span>
    </td>
  </tr>
</template>`;

document.head.appendChild($documentContainer);
