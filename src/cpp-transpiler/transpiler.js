import fs from 'fs'
import path from 'path'
import mustache, { render } from 'mustache'

mustache.escape = x => x // disable HTML escape

const STORE_TEMPLATE_PATH = path.join(__dirname, 'templates/store.mst')

function camel_to_snake(s)
{
  return s.split(/(?=[A-Z])/).join('_').toLowerCase()
}

function c_type(type)
{
  switch (type) {
    case 'number': return 'float'
    case 'string': return 'UBDL_STRING'
    default: {
      if (type.kind === 'list') {
        return `UBDL_LIST<${c_type(type.element_type)}>`
      } else {
        throw 'unexpected'
      }
    }
  }
}

function comma_it(a)
{
  if (a.length <= 1) {
    a = a.map(x => ({ ...x, comma: false }))
  } else {
    a = a.map(x => ({ ...x, comma: true }))
    a[a.length - 1].comma = false
  }
  return a
}

function map_property(p)
{
  return {
    prop_name: p.name,
    prop_snake_name: camel_to_snake(p.name),
    PROP_SNAKE_NAME: camel_to_snake(p.name).toUpperCase(),
    prop_type: c_type(p.type)
  }
}

function map_action(a)
{
  return {
    action_name: a.name,
    action_type_name: `${a.name[0].toUpperCase()}${a.name.slice(1)}`,
    action_snake_name: camel_to_snake(a.name),
    ACTION_SNAKE_NAME: camel_to_snake(a.name).toUpperCase(),
    params: comma_it(a.params.map(x => ({
      param_name: x.name,
      param_snake_name: camel_to_snake(x.name),
      param_type: c_type(x.type)
    })))
  }
}

function map_store(s)
{
  return {
    store_name: s.name,
    STORE_SNAKE_NAME: camel_to_snake(s.name).toUpperCase(),
    properties: comma_it(s.members.filter(x => x.kind === 'property').map(map_property)),
    actions: comma_it(s.members.filter(x => x.kind === 'action').map(map_action))
  }
}

export default function(ubdl)
{
  const template = fs.readFileSync(STORE_TEMPLATE_PATH).toString()
  return render(template, {
    module_dot_name: ubdl.module.join('.'),
    module: ubdl.module.join('_'),
    MODULE: ubdl.module.join('_').toUpperCase(),
    stores: comma_it(ubdl.declarations.filter(x => x.kind === 'store').map(map_store))
  })
}
