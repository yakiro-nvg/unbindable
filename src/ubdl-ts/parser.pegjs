start
  = module

module
  = _ 'module' _ module:module_name _ declarations:module_body {
    return { module, declarations }
  }

module_name
  = head:id tail:('.' id)* {
    return [head].concat(tail.map(x => x[1]))
  }

module_body
  = declarations:(_ module_decl _)* {
    return declarations.map(x => x[1])
  }

module_decl
  = store

store
  = 'store' _ name:id _ members:(_ store_decl _)* _ 'end' {
    return { kind: 'store', name, members: members.map(x => x[1]) }
  }

store_decl
  = property
  / action

property
  = name:id _ ':' _ type:type {
    return { kind: 'property', name, type }
  }

action
  = name:id _ '(' _ params:(_ parameter _)* _ ')' {
    return { kind: 'action', name, params: params.map(x => x[1]) }
  }

parameter
  = name:id _ ':' _ type:type {
    return { name, type }
  }

type
  = type_list
  / 'number'
  / 'string'

type_list
  = 'List' _ '<' _ element_type:type _ '>' {
    return { kind: 'list', element_type }
  }

id
  = head:char tail:id_part* {
    return head + tail.join('')
  }

id_part
  = char
  / digit
  / '_'

char
  = [a-zA-Z]

digit
  = [1-9]

_
  = (ws / comment)*

comment
  = multi_line_comment
  / single_line_comment

multi_line_comment
  = '/*' (!'*/' source_character)* '*/'

single_line_comment
  = '//' (!line_terminator source_character)*

line_terminator
  = [\n\r]

source_character
  = .

ws
  = [ \t\n\r]
