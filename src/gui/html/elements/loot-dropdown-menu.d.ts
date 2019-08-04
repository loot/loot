// The LootDropdownMenu class doesn't have a value property visible to TS
// because it gets added at runtime as part of its Polymer element init.
export default interface LootDropdownMenu extends HTMLElement {
  value: string;
  label: string;
}
