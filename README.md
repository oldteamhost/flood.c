# flood.c
Insanely fast stress-test L4, runs on TPACKETV3; supports many methods; control panel in HTML.
# config
A config contains two elements (1) instruction (2) comment.

## instruction
The instructions are as follows,
```
<name>=<value>;
```
It consists of 2 tokens, i.e. `=` and `;`. And two operands `name` `value`.
### name
`<name>` is the name (identifier) of the option to which `<value>` is assigned.
### value
`<value>` is what is assigned to `<name>`. However, it is worth noting that, “what is assigned” can be of different types; more specifically, `number`, `string`, `keyword`.

#### number
`<value>` of the first type (i.e. of type `number`), is a number, and its assignment looks like this, (here we assign 100, pps options),
```
pps=100;
```
#### string
`<value>` of the first second type (`string`), is any, string, and it is assigned like this,
```
target="google.com";
```
or
```
target='google.com';
```
That is, unlike `number`, a `<value>` of type `string` must already be enclosed in quotation marks, either single or double.
Also, the symbols `=`,`;`,`‘`,`’`,`"`,`\`, must be specified, before the symbol `\`; for they are `special characters`.
So instead of just `=` we write it like this,
```
target='\='; 
```
#### keyword
The third type `keyword`, represents any reserved word, `flood.c` perceives such words differently.
In total, there are 3 such words, `NULL`, `RAND`, `ARAND`.

The word `NULL` is specified as follows,
```
pps=NULL;
target=NULL;
```
This entry tells `flood.c` that the values of the `pps` and `target` options, should be default (or it should determine them automatically).

The second word, it's `ARAND`, the entry is like this,
```
source=ARAND;
```
Such an entry tells `flood.c` that the value of the `source` option, with each packet sent, should be a random value. That is, it must set the `source` parameter to a random value before sending EVERY packet.

The third word is `RAND`, it's written like this,
```
pps=RAND/1,10;
target=RAND/10,abcd
target=RAND/IPV4
target=RAND/DNS
```
This is where it's time to say that the options, `<name>`, can take either `number` and `keyword` or `string` and `keyword`.
For example, the `pps` option can only accept `<value>` of type - `number` or `keyword`. And the `target` option, values of type `string` or `keyword`.
So it will be said this way, the `pps` option is a `numeric option` and the `target` option is a `string option`.

So, here `numeric options` have this syntax `RAND`,
```
pps=RAND/1,10;
```
This tells `flood.c` that ONE SINGLE time at startup, it should set the `pps` option, as a value, to a random number between `1` and `10`.

And `string options` have this syntax `RAND`,
```
target=RAND/10,abcd
target=RAND/IPV4
target=RAND/DNS
```
The first entry tells `flood.c` ONE time at startup to specify a random string 10 characters long, from the abcd alphabet, as the value for the `target` field.
Second entry, specify a random IP 4 version address as the value.
Third entry, random DNS. (.com)

## comment
And the comment looks like this,
```
/* comment /* and comment */ */

/*
is
coom
  ** **** ent /* this */
*/
```
The beginning of a comment is asserted with the combination `/*` and the end of the comment with `*/`. Everything inside these characters is a comment.
