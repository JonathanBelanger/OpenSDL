# OpenSDL - A portable processor for Structure Definition Language (a rewrite of the OpenVMS SDL)

The Open Structure Definition Language (OpenSDL) is used to write source statements that describe data structures and that can be translated to source statements in other languages.  You can include the resulting output files in a corresponding target language program for subsequent compilation.

Because OpenSDL is compiler- and language-independent, it is particularly useful for maintaining multilanguage implementations.  For example, you can create and later modify a single OpenSDL source file that can be translated to multilanguage output files; any number of these output files can then be included in one or several multilanguage programming applications.

```
MODULE <id> IDENT <quoted_string> ;

CONSTANT <id> EQUALS <value> PREFIX <prefix> TAG <tag>;
  {
  { will generate a constant definition :
  {define <prefix>_K_<tag>_<id>	<value>
  {

CONSTANT (
  <id>,
  <id2>,
  <id3> = <value3>
  <id4>
) EUQALS 1 <value> PREFIX <prefix> TAG <tag> INCREMENT <inc> RADIX DEC|OCT|HEX|BIN;

  {will generate a set of definitions:
  {#define <prefix>_K_<tag>_<id>	<value>
  {#define <prefix>_K_<tag>_<id>	<value> + <inc>
  {#define <prefix>_K_<tag>_<id>	<value3>
  {#define <prefix>_K_<tag>_<id>	<value3>+<inc>
  {

LITERAL;
  Text to be copyied as-is;
END_LITERAL;

AGGREGATE <id>  STRUCTURE|UNION [agg_opts];
  <id>  <type_spec>;
END <id>;

  agg_opts:
    PREFIX <quoted_string>
    TAG <quoted_string>
    ALIGN [BYTE, WORD, LONG, QUAD, OCTA, PAGE]
    NOALIGN
    
  
  type_spec:
    BYTE, WORD, LONGWORD, QUADWORD, OCTAWORD, SIGNED, UNSIGNED, BITFIELD, DIMENSION
    STRUCT, UNION 
	
END_MODULE;
```
