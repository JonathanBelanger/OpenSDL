#SDL - A portable processor for Structure Definition Language (a child of the OpenVMS SDL)

A short description of the language follows:

MODULE <id> IDENT <quoted_string> ;

CONSTANT <id> EQUALS <value> PREFIX <prefix> TAG <tag>;
  {
  { will generate a constant definition :
  {define <prefix>_$K_<tag>_<id>	<value>
  {

CONSTANT (
  <id>,
  <id2>,
  <id3> = <value3>
  <id4>
) EUQALS 1 <value> PREFIX <prefix> TAG <tag> INCREMENT <inc> RADIX DEC|OCT|HEX|BIN;

  {will generate a set of definitions:
  {#define <prefix>_$K_<tag>_<id>	<value>
  {#define <prefix>_$K_<tag>_<id>	<value> + <inc>
  {#define <prefix>_$K_<tag>_<id>	<value3>
  {#define <prefix>_$K_<tag>_<id>	<value3>+<inc>
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
